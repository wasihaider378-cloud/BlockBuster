
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
    sf::Font font;

    if (!font.openFromFile("arial.ttf"))
    {
        cerr << "Error: Could not load font!" << endl;
        return 1;
    }
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
    int score = 0;
    int lives = 3;
    bool gameEnded = false;
    sf::Text scoreText(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition({ 20.f, 10.f });
    sf::Text livesText(font);
    livesText.setCharacterSize(24);
    livesText.setFillColor(sf::Color::White);
    livesText.setPosition({ 200.f, 10.f });
    sf::Text winText(font);
    winText.setCharacterSize(40);
    winText.setFillColor(sf::Color::Yellow);
    winText.setPosition({ 200.f, 300.f });
    sf::Text gameOverText(font);
    gameOverText.setCharacterSize(40);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setPosition({ 180.f, 300.f });
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
                if (key->code == sf::Keyboard::Key::Space &&
                    ballLaunched == false && gameEnded == false)
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
       if (gameEnded == false)
       {
            
        
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
       }
    
       if (ballLaunched == false)
       {
           ballX = paddleX + 44;
       }
       if (ballLaunched == true && gameEnded == false)
       {   
           bool alreadyHit[8][10] = {};

           ballX = ballX + dx;

           if (ballX < 0 || ballX + 12 > 600)
           {
               ballX = ballX - dx;
               dx = -dx;
           }
           float cornerX[4] = {
           ballX, ballX + 12, ballX, ballX + 12
           };

           float cornerY[4] = {
               ballY, ballY, ballY + 12, ballY + 12
           };
           bool brickTouched = false;

           for (int i = 0; i < 4; i++)
           {
               if (cornerY[i] >= 100 &&
                   cornerY[i] < 300 &&
                   cornerX[i] >= 0 &&
                   cornerX[i] < 600)
               {
                   int column = cornerX[i] / 60;
                   int row = (cornerY[i] - 100) / 25;
                   if (bricks[row][column] > 0)
                   {   
                       brickTouched = true;

                       if (alreadyHit[row][column] == false)
                       {
                           alreadyHit[row][column] = true;

                           bricks[row][column]--;

                           if (bricks[row][column] == 0)
                           {
                               score += 50;
                           }
                           else
                           {
                               score += 10;
                           }
                       }
                   }
               }
           } 
           if (brickTouched == true)
           {
               ballX = ballX - dx;
               dx = -dx;
           }
           
           ballY = ballY + dy;
           cornerX[0] = ballX;
           cornerX[1] = ballX + 12;
           cornerX[2] = ballX;
           cornerX[3] = ballX + 12;

           cornerY[0] = ballY;
           cornerY[1] = ballY;
           cornerY[2] = ballY + 12;
           cornerY[3] = ballY + 12;

           brickTouched = false;

           for (int i = 0; i < 4; i++)
           {
               if (cornerY[i] >= 100 &&
                   cornerY[i] < 300 &&
                   cornerX[i] >= 0 &&
                   cornerX[i] < 600)
               {
                   int column = cornerX[i] / 60;
                   int row = (cornerY[i] - 100) / 25;
                   if (bricks[row][column] > 0)
                   {
                       brickTouched = true;
                       if (alreadyHit[row][column] == false)
                       {
                           alreadyHit[row][column] = true;

                           bricks[row][column]--;

                           if (bricks[row][column] == 0)
                           {
                               score += 50;
                           }
                           else
                           {
                               score += 10;
                           }
                       }
                   }
               }
           }
           if (brickTouched == true)
           {
               ballY = ballY - dy;
               dy = -dy;
           }
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
       if (ballY > 700 && lives > 0)
       {
           lives--;

           ballLaunched = false;
           ballX = paddleX + 44;
           ballY = 638;
           dx = 0;
           dy = 0;
       }
       bool allBricksDestroyed = true;

       for (int r = 0; r < rows; r++)
       {
           for (int c = 0; c < cols; c++)
           {
               if (bricks[r][c] > 0)
               {
                   allBricksDestroyed = false;
               }
           }
       }
       if (allBricksDestroyed == true)
       {
           winText.setString("YOU WIN");
           gameEnded = true;
       }
       if (lives == 0)
       {
           gameOverText.setString("GAME OVER");
           gameEnded = true;
       }
        paddle.setPosition({ paddleX, 650.f });
        ball.setPosition({ ballX, ballY });
        window.draw(paddle);
        scoreText.setString("Score: " + to_string(score));
        window.draw(scoreText);
        livesText.setString("Lives: " + to_string(lives));
        window.draw(livesText);
        window.draw(winText);
        window.draw(gameOverText);
        window.draw(ball);
        window.display();
    }

    return 0;
}
