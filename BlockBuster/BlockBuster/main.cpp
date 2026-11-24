
#include <SFML/Graphics.hpp>
#include <optional>
#include <fstream>
#include <iostream>
using namespace std;
int main()
{
    sf::RenderWindow window(sf::VideoMode({ 600u, 700u }), "Block Buster");
    window.setFramerateLimit(60);
    sf::RectangleShape paddle;
    ifstream fin("level.txt");
    if (!fin)
    {
        cerr << "Error: Could not open level.txt!" << endl;
        return 1;
    }
    int rows = 0, cols = 0;
    if (!(fin >> rows >> cols))
    {
        cerr << "Error: Invalid level dimensions!" << endl;
        return 1;
    }
    if (rows != 8 || cols != 10)
    {
        cerr << "Error: Level must be 8 rows and 10 columns!" << endl;
        return 1;
    }
    int bricks[8][10];
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            if (!(fin >> bricks[r][c]))
            {
                cerr << "Error: Incomplete or invalid level data!" << endl;
                return 1;
            }
        }
    }
    const float BRICK_W = 60.f;
    const float BRICK_H = 25.f;
    const float BRICKS_Y = 100.f;
    sf::Color colors[4] = {
    sf::Color(25, 25, 45),
    sf::Color::Green,
    sf::Color::Yellow,
    sf::Color::Red
    };
    sf::RectangleShape brick({ BRICK_W - 2, BRICK_H - 2 });
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
        for (int r = 0; r < rows; r++)
        {
            for (int c = 0; c < cols; c++)
            {
                if (bricks[r][c] != 0)
                {
                    brick.setFillColor(colors[bricks[r][c]]);
                    brick.setPosition
                    ({
                               c * BRICK_W + 1,
                               BRICKS_Y + r * BRICK_H + 1
                    });
                    window.draw(brick);
                }
            }
        }

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
