
#include <SFML/Graphics.hpp>
#include <optional>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;
int main()
{  
    srand(time(0));
    sf::RenderWindow window(sf::VideoMode({ 600u, 700u }), "Block Buster");
    window.setFramerateLimit(60);
    sf::RectangleShape paddle;
    sf::CircleShape ball(6.f);
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
    float ballX = paddleX + 44;
    float ballY = 638;
    bool ballLaunched = false;
    float dx = 0.f;
    float dy = 0.f;
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
            else if (const auto* key = event->getIf<sf::Event::KeyPressed>())
            {
                if (key->code == sf::Keyboard::Key::Space && ballLaunched == false)
                {
                    ballLaunched = true;
                    dy = -5;
                    if (rand() % 2 == 0)
                    {
                        dx = -3;
                    }
                    else
                    {
                        dx = 3;
                    }
                }
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

        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) && paddleX <= 492)
        {
            paddleX = paddleX + 8;
        }
       if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) && paddleX >= 8)
       {
         paddleX = paddleX - 8;
       }
       if (ballLaunched == false)
       {
           ballX = paddleX + 44;
       }
       if (ballLaunched == true)
       {
           ballX = ballX + dx;

           if (ballX < 0 || ballX + 12 > 600)
           {
               ballX = ballX - dx;
               dx = -dx;
           }
           ballY = ballY + dy;
           if (ballY < 50)
           {
               ballY = ballY - dy;
               dy = -dy;
           }
           if (dy > 0 &&
               ballX < paddleX + 100 &&
               paddleX < ballX + 12 &&
               ballY + 12 > 650 &&
               ballY < 664)
           {
               ballY = 638;
               dy = -5;

               float sectionValue = (ballX + 6 - paddleX) / 25;

               if (sectionValue < 0)
               {
                   sectionValue = 0;
               }

               if (sectionValue > 3)
               {
                   sectionValue = 3;
               }

               int section = static_cast<int>(sectionValue);

               if (section == 0)
               {
                   dx = -6;
               }
               else if (section == 1)
               {
                   dx = -3;
               }
               else if (section == 2)
               {
                   dx = 3;
               }
               else if (section == 3)
               {
                   dx = 6;
               }
           }
       }
        paddle.setPosition({ paddleX, 650.f });
        ball.setPosition({ ballX, ballY });
        window.draw(paddle);
        window.draw(ball);
        window.display();
    }

    return 0;
}
