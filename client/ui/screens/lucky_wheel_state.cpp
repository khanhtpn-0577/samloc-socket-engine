#include "lucky_wheel_state.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>


LuckyWheelState::LuckyWheelState(StateContext& ctx)
    : ctx_(ctx),
      currentRotation_(0.f),
      rotationSpeed_(0.f),
      spinning_(false),
      waitingForServer_(false),
      targetRewardIndex_(0),
      targetRotation_(0.f),
      hasServerResult_(false),
      slowdownTimer_(0.f) {

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // ===== Background =====
    background_.setSize({1280.f, 720.f});
    background_.setFillColor(sf::Color(30, 20, 40));

    // ===== Title =====
    titleText_.setFont(ctx_.font);
    titleText_.setString("Lucky Wheel");
    titleText_.setCharacterSize(48);
    titleText_.setFillColor(sf::Color(255, 215, 0));
    titleText_.setStyle(sf::Text::Bold);
    sf::FloatRect titleBounds = titleText_.getLocalBounds();
    titleText_.setOrigin(titleBounds.width / 2.f, titleBounds.height / 2.f);
    titleText_.setPosition(640.f, 50.f);

    // ===== Balance Info =====
    balanceText_.setFont(ctx_.font);
    balanceText_.setCharacterSize(24);
    balanceText_.setFillColor(sf::Color(255, 215, 0));
    balanceText_.setStyle(sf::Text::Bold);
    balanceText_.setPosition(40.f, 40.f);

    // ===== Wheel Container =====
    wheelCenter_ = {640.f, 360.f};

    // ===== Spin Button - CẬP NHẬT callback =====
    spinButton_.setFont(ctx_.font);
    spinButton_.setText("SPIN", 28);
    spinButton_.setSize({180.f, 60.f});
    spinButton_.setPosition({550.f, 600.f});
    spinButton_.setColors(
        sf::Color(0, 180, 0),
        sf::Color::White,
        sf::Color::White
    );

    spinButton_.setCallback([this]() {
        // Chỉ cho phép spin khi không đang quay và không đang chờ server
        if (!spinning_ && !waitingForServer_) {
            // Bắt đầu quay ngay
            spinning_ = true;
            waitingForServer_ = true;
            hasServerResult_ = false;
            rotationSpeed_ = 1200.f; // Tốc độ quay ban đầu cao
            
            std::cout << "[LuckyWheel] Starting spin and requesting from server...\n";
            
            // GỌI HÀM HANDLER ĐỂ GỬI REQUEST LÊN SERVER
            ctx_.luckyWheelHandler.requestSpin();
            
            // Có thể thêm hiệu ứng loading ở đây
            spinButton_.setText("SPINNING...", 22);
        }
    });

    // ===== Back Button =====
    backButton_.setFont(ctx_.font);
    backButton_.setText("Back", 20);
    backButton_.setSize({140.f, 50.f});
    backButton_.setPosition({40.f, 640.f});
    backButton_.setColors(
        sf::Color(80, 80, 120),
        sf::Color::White,
        sf::Color::White
    );
    backButton_.setCallback([this]() {
        ctx_.requestTransition(GameStateType::Lobby);
    });

    initRewards();
}

void LuckyWheelState::initRewards() {
    rewards_ = {
        "1k coin",
        "10k coin", 
        "20k coin",
        "50k coin",
        "75k coin",
        "100k coin",
        "200k coin",
        "1M coin"
    };

    wheelSectors_.clear();
    rewardTexts_.clear();

    const float radius = 220.f;
    const float angleStep = 360.f / rewards_.size();

    // Màu sắc đẹp mắt cho các sector
    std::vector<sf::Color> colors = {
        sf::Color(255, 59, 59),   // Red
        sf::Color(255, 149, 0),   // Orange
        sf::Color(255, 204, 0),   // Yellow
        sf::Color(76, 217, 100),  // Green
        sf::Color(90, 200, 250),  // Light Blue
        sf::Color(52, 120, 246),  // Blue
        sf::Color(175, 82, 222),  // Purple
        sf::Color(255, 45, 85)    // Pink
    };

    for (size_t i = 0; i < rewards_.size(); ++i) {
        // Tạo sector với nhiều điểm hơn để mượt hơn
        sf::ConvexShape sector;
        sector.setPointCount(50);
        
        // Điểm trung tâm
        sector.setPoint(0, sf::Vector2f(0.f, 0.f));
        
        // Tính toán góc bắt đầu và kết thúc
        float startAngleDeg = angleStep * i;
        float endAngleDeg = angleStep * (i + 1);
        
        // Tạo các điểm trên cung tròn
        for (int j = 0; j < 49; ++j) {
            float angle = startAngleDeg + (endAngleDeg - startAngleDeg) * j / 48.f;
            float angleRad = (angle - 90.f) * 3.1415926f / 180.f;
            
            sector.setPoint(j + 1, sf::Vector2f(
                radius * std::cos(angleRad),
                radius * std::sin(angleRad)
            ));
        }
        
        sector.setFillColor(colors[i % colors.size()]);
        sector.setOutlineThickness(2.f);
        sector.setOutlineColor(sf::Color(255, 215, 0, 150));
        
        wheelSectors_.push_back(sector);

        // Tạo text cho phần thưởng
        sf::Text text;
        text.setFont(ctx_.font);
        text.setString(rewards_[i]);
        text.setCharacterSize(20);
        text.setFillColor(sf::Color::White);
        text.setStyle(sf::Text::Bold);
        
        text.setOutlineThickness(1.5f);
        text.setOutlineColor(sf::Color::Black);

        float midAngleDeg = startAngleDeg + angleStep / 2.f;
        float midAngleRad = (midAngleDeg - 90.f) * 3.1415926f / 180.f;
        float textRadius = radius * 0.68f;

        float x = textRadius * std::cos(midAngleRad);
        float y = textRadius * std::sin(midAngleRad);

        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
        text.setPosition(x, y);
        text.setRotation(midAngleDeg);

        rewardTexts_.push_back(text);
    }

    // Tạo viền ngoài của wheel
    outerCircle_.setRadius(radius);
    outerCircle_.setPointCount(100);
    outerCircle_.setFillColor(sf::Color::Transparent);
    outerCircle_.setOutlineThickness(6.f);
    outerCircle_.setOutlineColor(sf::Color(255, 215, 0));
    outerCircle_.setOrigin(radius, radius);

    // Tạo vòng tròn trung tâm
    centerCircle_.setRadius(40.f);
    centerCircle_.setPointCount(50);
    centerCircle_.setFillColor(sf::Color(255, 215, 0));
    centerCircle_.setOutlineThickness(4.f);
    centerCircle_.setOutlineColor(sf::Color::White);
    centerCircle_.setOrigin(40.f, 40.f);
}

void LuckyWheelState::onEnter() {
    std::cout << "[LuckyWheelState] Entered\n";

    // ===== HIỂN THỊ BALANCE NGAY KHI VÀO MÀN =====
    // Giống logic bên LobbyState
    balanceText_.setString(
        "Balance: " + std::to_string(
            static_cast<long long>(ctx_.session.balance())
        )
    );

    // ===== ĐĂNG KÝ CALLBACK ĐỂ NHẬN KẾT QUẢ TỪ SERVER =====
    ctx_.luckyWheelHandler.setSpinResultCallback(
        [this](const LuckyWheelResult& result) {
            std::cout << "[LuckyWheel] ✓ Received result from server:\n"
                      << "  - Name: " << result.rewardName << "\n"
                      << "  - Reward: " << result.rewardName << "\n"
                      << "  - Amount: " << result.rewardAmount << "\n"
                      << "  - New Balance: " << result.newBalance << "\n";

            // ===== UPDATE BALANCE TEXT KHI CÓ KẾT QUẢ =====
            pendingNewBalance_ = result.newBalance;

            // Nếu spin thất bại (ví dụ: không đủ tiền) thì dừng luôn
            if (!result.success) {
                waitingForServer_ = false;
                spinning_ = false;
                hasServerResult_ = false;
                rotationSpeed_ = 0.f;
                spinButton_.setText("SPIN", 28);
                return;
            }

            // ===== LOGIC QUAY WHEEL =====
            waitingForServer_ = false;
            hasServerResult_ = true;
            slowdownTimer_ = 2.0f; // Quay thêm 2 giây

            auto it = std::find(
                rewards_.begin(),
                rewards_.end(),
                result.rewardName
            );

            if (it == rewards_.end()) {
                std::cerr << "[LuckyWheel] Unknown reward from server: "
                          << result.rewardName << "\n";
                return;
            }

            std::cout << "[LuckyWheel Mapping Result] Stopping at reward: "
                      << *it << "\n";

            targetRewardIndex_ =
                static_cast<int>(std::distance(rewards_.begin(), it));

            float angleStep = 360.f / rewards_.size();

            float currentNormalized = std::fmod(currentRotation_, 360.f);
            if (currentNormalized < 0) currentNormalized += 360.f;

            // --- SỬA LOGIC TẠI ĐÂY ---
            // Vì hình vẽ sector đã được xoay -90 độ (để 0 nằm ở đỉnh/mũi tên), 
            // nên ta dùng 360 (tức 0) làm mốc thay vì 270.
            float targetAngle =
                360.f - (targetRewardIndex_ * angleStep + angleStep / 2.f);

            while (targetAngle < 0) targetAngle += 360.f;
            while (targetAngle >= 360.f) targetAngle -= 360.f;

            float angleDiff = targetAngle - currentNormalized;
            if (angleDiff < 0) angleDiff += 360.f;

            float extraRotations = 2.f + (std::rand() % 2);
            targetRotation_ =
                currentRotation_ + extraRotations * 360.f + angleDiff;

            std::cout << "[LuckyWheel] Will stop at index "
                      << targetRewardIndex_
                      << " after 2 more seconds (target rotation: "
                      << targetRotation_ << "°)\n";
        }
    );
}


void LuckyWheelState::onExit() {
    std::cout << "[LuckyWheelState] Exited\n";
}

void LuckyWheelState::handleEvent(
    const sf::Event& event,
    const sf::Vector2f& mousePos
) {
    spinButton_.handleEvent(event, mousePos);
    backButton_.handleEvent(event, mousePos);
}

void LuckyWheelState::update(float dt) {
    if (!spinning_) return;

    // Nếu chưa nhận kết quả từ server, quay với tốc độ cao
    if (!hasServerResult_) {
        // Quay nhanh và duy trì tốc độ
        currentRotation_ += rotationSpeed_ * dt;
        
        // Giữ tốc độ ổn định (không giảm) khi đang chờ server
        // Có thể dao động nhẹ để tự nhiên hơn
        rotationSpeed_ = 1000.f + std::sin(currentRotation_ * 0.01f) * 100.f;
        
        return;
    }
    
    // Đã nhận được kết quả từ server, bắt đầu đếm ngược 2 giây
    slowdownTimer_ -= dt;
    
    if (slowdownTimer_ > 0.f) {
        // Còn thời gian, tiếp tục quay với tốc độ cao
        currentRotation_ += rotationSpeed_ * dt;
        
        // Giảm tốc độ dần trong 2 giây
        float progress = 1.f - (slowdownTimer_ / 2.f); // 0 -> 1
        rotationSpeed_ = 1000.f * (1.f - progress * 0.5f); // Giảm từ 1000 -> 500
    } else {
        // Hết 2 giây, bắt đầu giảm tốc mạnh để dừng đúng vị trí
        float remainingAngle = targetRotation_ - currentRotation_;
        
        if (remainingAngle > 0.1f) {
            // Còn xa target, tiếp tục quay với tốc độ giảm dần
            rotationSpeed_ -= 600.f * dt;
            
            if (rotationSpeed_ < 100.f) {
                rotationSpeed_ = 100.f; // Tốc độ tối thiểu
            }
            
            currentRotation_ += rotationSpeed_ * dt;
            
            // Nếu gần target, điều chỉnh tốc độ để dừng đúng
            if (remainingAngle < 50.f) {
                rotationSpeed_ = remainingAngle * 2.f; // Tốc độ tỉ lệ với khoảng cách
            }
        } else {
            // Đã đến target, dừng hẳn
            spinning_ = false;
            rotationSpeed_ = 0.f;
            currentRotation_ = targetRotation_;
            hasServerResult_ = false;
            
            // Chuẩn hóa góc để hiển thị
            float normalizedRotation = std::fmod(currentRotation_, 360.f);
            if (normalizedRotation < 0) normalizedRotation += 360.f;
            
            std::cout << "[LuckyWheel] ✓ Stopped at reward: " 
                      << rewards_[targetRewardIndex_] << "\n"
                      << "  - Final rotation: " << normalizedRotation << "°\n";
            
            // Reset button text
            spinButton_.setText("SPIN", 28);

            // ===== UPDATE BALANCE SAU KHI QUAY XONG =====
            ctx_.session.setBalance(pendingNewBalance_);

            balanceText_.setString(
                "Balance: " + std::to_string(pendingNewBalance_)
            );

        }
    }
}

void LuckyWheelState::draw(sf::RenderWindow& window) {
    window.draw(background_);
    window.draw(titleText_);
    window.draw(balanceText_);

    // Vẽ tất cả các phần của wheel với cùng một rotation
    sf::Transform transform;
    transform.translate(wheelCenter_);
    transform.rotate(currentRotation_);

    // Vẽ các sectors
    for (const auto& sector : wheelSectors_) {
        window.draw(sector, transform);
    }

    // Vẽ viền ngoài
    window.draw(outerCircle_, transform);

    // Vẽ các text
    for (const auto& text : rewardTexts_) {
        window.draw(text, transform);
    }

    // Vẽ vòng tròn trung tâm
    window.draw(centerCircle_, transform);

    // Vẽ mũi tên chỉ vị trí trúng thưởng
    sf::ConvexShape pointer;
    pointer.setPointCount(7);
    
    pointer.setPoint(0, sf::Vector2f(0.f, 35.f));
    pointer.setPoint(1, sf::Vector2f(-15.f, 10.f));
    pointer.setPoint(2, sf::Vector2f(-8.f, 10.f));
    pointer.setPoint(3, sf::Vector2f(-8.f, -20.f));
    pointer.setPoint(4, sf::Vector2f(8.f, -20.f));
    pointer.setPoint(5, sf::Vector2f(8.f, 10.f));
    pointer.setPoint(6, sf::Vector2f(15.f, 10.f));
    
    pointer.setFillColor(sf::Color(255, 215, 0));
    pointer.setOutlineThickness(3.f);
    pointer.setOutlineColor(sf::Color(255, 255, 255));
    pointer.setPosition(wheelCenter_.x, wheelCenter_.y - 240.f);
    window.draw(pointer);

    spinButton_.draw(window);
    backButton_.draw(window);
}