#include "card_ui.h"
#include <iostream>

void CardUI::centerText(sf::Text& txt, float offsetX, float offsetY) {
    sf::FloatRect textRect = txt.getLocalBounds();
    txt.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    txt.setPosition(offsetX, offsetY);
}

void CardUI::setupText(sf::Text& txt, sf::String str, int size, sf::Color color, float x, float y) {
    txt.setFont(*m_font);
    txt.setString(str);
    txt.setCharacterSize(size);
    txt.setFillColor(color);
    txt.setPosition(x, y);
}

CardUI::CardUI(int id, sf::Font& font) 
    : m_font(&font), m_isSelected(false), m_isHovered(false) 
{
    decodeID(id);
    setupUI();
}

void CardUI::decodeID(int id) {
    m_info.id = id;
    if (id < 1) return;

    int idx = id - 1;
    int suit = idx / 13; // 0:Bích, 1:Chuồn, 2:Rô, 3:Cơ
    int rank = idx % 13; // 0:3, ..., 12:2

    m_info.color = (suit >= 2) ? sf::Color(231, 76, 60) : sf::Color(44, 62, 80);
    
    std::wstring symbols[] = { L"\u2660", L"\u2663", L"\u2666", L"\u2665" };
    m_info.symbol = symbols[suit];

    std::string ranks[] = {"3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A", "2"};
    m_info.rank = ranks[rank];
}

void CardUI::setupUI() {
    sf::Vector2f defaultSize(100.f, 140.f);
    m_shape.setSize(defaultSize);
    m_shape.setFillColor(sf::Color::White);
    m_shape.setOutlineThickness(2.f);
    m_shape.setOutlineColor(sf::Color(180, 180, 180));

    // Shadow (Bóng đổ nhẹ)
    m_shadow.setSize(defaultSize);
    m_shadow.setFillColor(sf::Color(0,0,0,50));
    m_shadow.setPosition(4, 4);

    // Rank & Suit ở góc (Ghi số và ký hiệu nhỏ)
    setupText(m_txtCorner, sf::String(m_info.rank), 22, m_info.color, defaultSize.x * 0.1f, defaultSize.y * 0.05f);
    setupText(m_txtSuitSmall, sf::String(m_info.symbol), 18, m_info.color, defaultSize.x * 0.1f, defaultSize.y * 0.22f);

    // Hình vẽ chính ở giữa
    // Nếu là bài Tây (J, Q, K, A) thì hiện chữ lớn, nếu là bài số thì hiện chất lớn
    bool isFace = (m_info.rank == "J" || m_info.rank == "Q" || m_info.rank == "K" || m_info.rank == "A" || m_info.rank == "2");
    sf::String mainStr = isFace ? sf::String(m_info.rank) : sf::String(m_info.symbol);
    
    setupText(m_txtMain, mainStr, isFace ? 50 : 60, m_info.color, defaultSize.x / 2.0f, defaultSize.y / 2.0f);
    centerText(m_txtMain, defaultSize.x / 2.0f, defaultSize.y / 2.0f);
}

void CardUI::setSize(const sf::Vector2f& size) {
    m_shape.setSize(size);
    m_shadow.setSize(size);
    
    float scaleFactor = size.y / 140.f;
    
    setupText(m_txtCorner, sf::String(m_info.rank), (int)(22 * scaleFactor), m_info.color, size.x * 0.1f, size.y * 0.05f);
    setupText(m_txtSuitSmall, sf::String(m_info.symbol), (int)(18 * scaleFactor), m_info.color, size.x * 0.1f, size.y * 0.22f);

    bool isFace = (m_info.rank == "J" || m_info.rank == "Q" || m_info.rank == "K" || m_info.rank == "A" || m_info.rank == "2");
    sf::String mainStr = isFace ? sf::String(m_info.rank) : sf::String(m_info.symbol);
    setupText(m_txtMain, mainStr, (int)((isFace ? 50 : 60) * scaleFactor), m_info.color, size.x / 2.0f, size.y / 2.0f);
    centerText(m_txtMain, size.x / 2.0f, size.y / 2.0f);
}

bool CardUI::contains(float x, float y) const {
    sf::FloatRect bounds = m_shape.getGlobalBounds();
    // Điều chỉnh bounds theo vị trí hiện tại và trạng thái nhô lên (hover/select)
    sf::Vector2f pos = getPosition();
    float topOffset = 0;
    if (m_isSelected) topOffset = -40;
    else if (m_isHovered) topOffset = -20;

    bounds.left = pos.x;
    bounds.top = pos.y + topOffset;
    
    return bounds.contains(x, y);
}

void CardUI::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    
    // Hiệu ứng nhô lên khi chọn hoặc hover
    if (m_isSelected) states.transform.translate(0, -40);
    else if (m_isHovered) states.transform.translate(0, -20);

    target.draw(m_shadow, states);
    target.draw(m_shape, states);
    target.draw(m_txtCorner, states);
    target.draw(m_txtSuitSmall, states);
    target.draw(m_txtMain, states);
}

int CardUI::getID() const { return m_info.id; }
bool CardUI::isSelected() const { return m_isSelected; }
void CardUI::toggleSelect() { m_isSelected = !m_isSelected; }
void CardUI::setHover(bool state) { m_isHovered = state; }