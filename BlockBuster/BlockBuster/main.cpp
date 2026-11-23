
#include <SFML/Graphics.hpp>
#include <optional>

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 600u, 700u }), "Block Buster - SFML Test");
    window.setFramerateLimit(60);
    sf::RectangleShape paddle;

    float paddleX = 250.f;
    paddle.setSize({ 100.f, 14.f });
    paddle.setPosition({ paddleX, 650.f });
    paddle.setFillColor(sf::Color::Green);
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }
        window.clear(sf::Color::Black);

     if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) && paddleX<=492)
     {
        paddleX = paddleX + 8;
     }
     if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) && paddleX >= 8)
     {
         paddleX = paddleX - 8;
     }
         paddle.setPosition({ paddleX, 650.f });
        window.draw(paddle);
        window.display();
    }

    return 0;
}
