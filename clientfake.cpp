#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <cstring>
#include <chrono>
#include <ctime>
#include <sstream>
#include <algorithm>
#include <limits>

// --- OS Specific Includes ---
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

// --- CONFIG ---
const char* SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 5000;

std::mutex coutMutex;

// --- PROTOCOL DEFINITIONS ---
// Copy enum này khớp với server/net/protocol.h hoặc game_handler.h
enum class MessageType : uint16_t {
    // System / Room
    C_GET_ROOM_LIST = 0x2000,
    S_ROOM_LIST     = 0x2001,
    C_JOIN_ROOM     = 0x2002,
    S_PLAYER_JOINED = 0x2003,
    C_LEAVE_ROOM    = 0x2004, // Server dùng LEAVE_ROOM hoặc C_LEAVE_ROOM
    S_PLAYER_LEFT   = 0x2005,
    C_READY         = 0x2006,
    S_PLAYER_READY  = 0x2007,
    
    // Game Logic
    C_BAO_SAM       = 0x3013, // Kiểm tra lại protocol server, giả sử 0x3013
    C_PLAY_CARD     = 0x3011,
    C_PASS_TURN     = 0x3012,
    
    // Error
    ERROR_MESSAGE   = 0xE001
};

#pragma pack(push, 1)
struct MessageHeader {
    uint16_t messageType;
    uint32_t senderId;
    uint64_t timestamp;
    uint8_t  token[32];
    uint32_t payloadLength;
    
    MessageHeader() { 
        messageType=0; senderId=0; timestamp=0; payloadLength=0; 
        std::memset(token,0,sizeof(token)); 
    }
};
#pragma pack(pop)

// --- HELPER FUNCTIONS ---

// Gửi toàn bộ dữ liệu (tránh partial send)
bool sendAll(int sock, const void* data, size_t size) {
    const char* p = (const char*)data;
    size_t sent = 0;
    while (sent < size) {
        ssize_t n = send(sock, p + sent, size - sent, 0);
        if (n <= 0) return false;
        sent += n;
    }
    return true;
}

// Đóng gói và gửi message kèm Header
void sendMessage(int sock, MessageType type, int userId, const std::string& payload = "") {
    MessageHeader h;
    h.messageType = static_cast<uint16_t>(type);
    h.senderId = userId;
    h.timestamp = std::time(nullptr);
    h.payloadLength = payload.size();

    // 1. Gửi Header
    sendAll(sock, &h, sizeof(h));
    
    // 2. Gửi Payload (nếu có)
    if (!payload.empty()) sendAll(sock, payload.data(), payload.size());

    // Log ra màn hình console của client
    std::lock_guard<std::mutex> lock(coutMutex);
    std::cout << "\033[1;32m[SEND] >>> \033[0m Type=0x" << std::hex << h.messageType << std::dec 
              << " Payload=" << (payload.empty() ? "(empty)" : payload) << "\n";
}

// Thread nhận tin nhắn từ server
void recvThread(int sock) {
    std::vector<uint8_t> buffer; buffer.reserve(8192);
    while (true) {
        uint8_t temp[4096];
        ssize_t n = recv(sock, temp, sizeof(temp), 0);
        if (n > 0) { 
            buffer.insert(buffer.end(), temp, temp + n); 
        } else if (n == 0) { 
            std::cout << "\n\033[1;31m[CLIENT] Server connection closed.\033[0m\n"; 
            exit(0); 
        } else { 
            if (errno==EAGAIN||errno==EWOULDBLOCK) continue; 
            perror("recv"); exit(1); 
        }

        // Xử lý gói tin (TCP Stream handling)
        while (true) {
            if (buffer.size() < sizeof(MessageHeader)) break;
            
            MessageHeader header;
            std::memcpy(&header, buffer.data(), sizeof(MessageHeader));
            size_t totalSize = sizeof(MessageHeader) + header.payloadLength;
            
            if (buffer.size() < totalSize) break; // Chưa đủ data payload

            std::string payload;
            if (header.payloadLength > 0) {
                payload.assign((char*)buffer.data() + sizeof(MessageHeader), header.payloadLength);
            }

            // In tin nhắn nhận được
            {
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cout << "\n\033[1;36m[RECV] <<< \033[0m Type=0x" << std::hex << header.messageType << std::dec << "\n";
                std::cout << "             Payload: " << payload << "\n";
                std::cout << "\033[1;33mCommand > \033[0m" << std::flush;
            }

            buffer.erase(buffer.begin(), buffer.begin() + totalSize);
        }
    }
}

// Helper tạo JSON mảng bài
std::string makeCardsJson(const std::vector<int>& cards) {
    std::stringstream ss; ss << "{\"cards\":[";
    for (size_t i = 0; i < cards.size(); ++i) { 
        ss << cards[i] << (i+1<cards.size() ? "," : ""); 
    }
    ss << "]}";
    return ss.str();
}

// --- MAIN MENU ---
void showMenu() {
    std::lock_guard<std::mutex> lock(coutMutex);
    std::cout << "\n================ SAM LOC TEST CLIENT ================\n";
    std::cout << "1.  Get Room List   (Xem danh sach phong)\n";
    std::cout << "2.  Join Room       (Vao phong theo ID)\n";
    std::cout << "3.  Ready / Unready (San sang / Huy san sang)\n";
    std::cout << "4.  Bao Sam         (Xin bao sam / Huy bao sam)\n";
    std::cout << "5.  Play Cards      (Danh bai - Nhap ID la bai)\n";
    std::cout << "6.  Pass Turn       (Bo luot)\n";
    std::cout << "7.  Leave Room      (Roi phong / Disconnect gia lap)\n";
    std::cout << "0.  Exit\n";
    std::cout << "=====================================================\n";
    std::cout << "\033[1;33mCommand > \033[0m" << std::flush;
}

int main(int argc, char* argv[]) {
    if (argc < 2) { std::cout << "Usage: ./client <USER_ID>\n"; return 1; }
    int userId = std::stoi(argv[1]);

    // 1. Kết nối
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return 1;
    sockaddr_in addr{}; addr.sin_family = AF_INET; addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);
    
    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) { 
        perror("Connect failed"); 
        std::cout << "Make sure server is running on port " << SERVER_PORT << "\n";
        return 1; 
    }

    std::cout << "[CLIENT] Connected successfully as User " << userId << "\n";
    
    // 2. Chạy luồng nhận tin
    std::thread(recvThread, sock).detach();

    // 3. Loop xử lý lệnh
    while (true) {
        showMenu();
        int cmd; 
        if (!(std::cin >> cmd)) { // Fix lỗi lặp vô tận nếu nhập chữ
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        if (cmd == 0) break;

        // Xử lý từng lệnh
        switch (cmd) {
            case 1: {
                /**
                 * [DOCS] Get Room List
                 * MSG_TYPE: C_GET_ROOM_LIST (0x2000)
                 * PAYLOAD:  "" (Empty) hoặc "{}"
                 */
                sendMessage(sock, MessageType::C_GET_ROOM_LIST, userId);
                break;
            }

            case 2: {
                /**
                 * [DOCS] Join Room
                 * MSG_TYPE: C_JOIN_ROOM (0x2002)
                 * PAYLOAD:  {"roomId": <int>}
                 */
                int rid;
                std::cout << "Enter Room ID to join: "; std::cin >> rid;
                std::string payload = "{\"roomId\":" + std::to_string(rid) + "}";
                sendMessage(sock, MessageType::C_JOIN_ROOM, userId, payload);
                break;
            }

            case 3: {
                /**
                 * [DOCS] Ready
                 * MSG_TYPE: C_READY (0x2006)
                 * PAYLOAD:  {"isReady": true} hoặc {"isReady": false}
                 */
                char opt; std::cout << "Set Ready? (y/n): "; std::cin >> opt;
                bool isReady = (opt == 'y' || opt == 'Y');
                std::string payload = isReady ? "{\"isReady\":true}" : "{\"isReady\":false}";
                sendMessage(sock, MessageType::C_READY, userId, payload);
                break;
            }

            case 4: {
                /**
                 * [DOCS] Bao Sam
                 * MSG_TYPE: C_BAO_SAM (Check enum)
                 * PAYLOAD:  {"wantSam": true} hoặc {"wantSam": false}
                 */
                char opt; std::cout << "Want Bao Sam? (y/n): "; std::cin >> opt;
                bool want = (opt == 'y' || opt == 'Y');
                std::string payload = want ? "{\"wantSam\":true}" : "{\"wantSam\":false}";
                sendMessage(sock, MessageType::C_BAO_SAM, userId, payload);
                break;
            }

            case 5: {
                /**
                 * [DOCS] Play Card
                 * MSG_TYPE: C_PLAY_CARD (0x3011)
                 * PAYLOAD:  {"cards": [id1, id2, ...]}
                 * INPUT:    Nhập các số cách nhau bởi dấu phẩy hoặc khoảng trắng
                 */
                std::cout << "Enter Card IDs (e.g. 15 16 17): ";
                // Xóa buffer cũ
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                
                std::string line;
                std::getline(std::cin, line);
                
                // Parse string thành vector int
                std::stringstream ss(line);
                std::vector<int> cards;
                int tempVal;
                while (ss >> tempVal) {
                    cards.push_back(tempVal);
                    // Bỏ qua dấu phẩy nếu người dùng nhập kiểu 1,2,3
                    if (ss.peek() == ',' || ss.peek() == ' ') ss.ignore();
                }

                if (cards.empty()) {
                    std::cout << "No cards selected!\n";
                } else {
                    std::string payload = makeCardsJson(cards);
                    sendMessage(sock, MessageType::C_PLAY_CARD, userId, payload);
                }
                break;
            }

            case 6: {
                /**
                 * [DOCS] Pass Turn
                 * MSG_TYPE: C_PASS_TURN (0x3012)
                 * PAYLOAD:  "{}" (Empty JSON) hoặc rỗng
                 */
                sendMessage(sock, MessageType::C_PASS_TURN, userId, "{}");
                break;
            }

            case 7: {
                /**
                 * [DOCS] Leave Room
                 * MSG_TYPE: C_LEAVE_ROOM (0x2004)
                 * PAYLOAD:  "{}"
                 */
                sendMessage(sock, MessageType::C_LEAVE_ROOM, userId, "{}");
                break;
            }

            default:
                std::cout << "Invalid command!\n";
                break;
        }

        // Delay một chút để log hiển thị đẹp hơn
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    close(sock);
    return 0;
}