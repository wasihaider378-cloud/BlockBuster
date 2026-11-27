
#include <SFML/Graphics.hpp>
#include <optional>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;
const int MENU = 0;
const int PLAYING = 1;
const int GAME_OVER = 2;

// Game dimensions and settings
const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 700;
const int FRAME_LIMIT = 60;
const int BRICK_ROWS = 8;
const int BRICK_COLS = 10;
const float BRICK_WIDTH = 60.f;
const float BRICK_HEIGHT = 25.f;
const float BRICKS_Y = 100.f;
const float BRICK_AREA_BOTTOM = 300.f;
const float PADDLE_START_X = 250.f;
const float PADDLE_Y = 650.f;
const float BALL_START_Y = 638.f;
const float PADDLE_MARGIN = 8.f;
const float BALL_TOP_LIMIT = 50.f;
const int INITIAL_LIVES = 3;

// Paddle settings
const float PADDLE_WIDTH = 100.f;
const float WIDE_PADDLE_WIDTH = 150.f;
const float PADDLE_SPEED = 8.f;

// Ball settings
const float BALL_SIZE = 12.f;
const float BALL_LAUNCH_SPEED_X = 3.f;
const float BALL_LAUNCH_SPEED_Y = -5.f;
const float BALL_FAST_SPEED_X = 6.f;

// Scoring
const int BRICK_HIT_POINTS = 10;
const int BRICK_BREAK_POINTS = 50;

// Power-up settings
const float POWERUP_FALL_SPEED = 3.f;
const int WIDE_PADDLE_DURATION = 600;
const float POWERUP_SIZE = 24.f;
const int POWERUP_CHANCE = 5;
const float PADDLE_HEIGHT = 14.f;

int gameState = MENU;
int menuSelection = 0;

// Power-up variables
float currentPaddleWidth = PADDLE_WIDTH;
int widePaddleTimer = 0;

bool powerUpActive = false;
char powerUpType = ' ';
float powerUpX = 0.f;
float powerUpY = 0.f;

// Randomly spawns a power-up at the position of a broken brick if none is currently active.
void spawnPowerUp(int row, int column)
{
    if (powerUpActive == false &&
        rand() % POWERUP_CHANCE == 0)
    {
        powerUpActive = true;

        if (rand() % 2 == 0)
        {
            powerUpType = 'W';
        }
        else
        {
            powerUpType = '+';
        }

        powerUpX = column * BRICK_WIDTH +
            (BRICK_WIDTH - POWERUP_SIZE) / 2;

        powerUpY = BRICKS_Y + row * BRICK_HEIGHT;
    }
}

// Reads a complete level file before replacing the current brick layout.
bool loadLevel(int bricks[BRICK_ROWS][BRICK_COLS], int rows, int cols)
{
    ifstream levelFile("level.txt");
    int fileRows = 0;
    int fileCols = 0;
    int freshBricks[BRICK_ROWS][BRICK_COLS];

    if (!levelFile ||
        !(levelFile >> fileRows >> fileCols) ||
        fileRows != rows || fileCols != cols)
    {
        return false;
    }

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            if (!(levelFile >> freshBricks[r][c]))
            {
                return false;
            }
        }
    }

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            bricks[r][c] = freshBricks[r][c];
        }
    }

    return true;
}

// Loads and validates the saved score, lives, and brick layout.
bool loadSavedGame(
    int bricks[BRICK_ROWS][BRICK_COLS],
    int rows, int cols, int& score, int& lives
)
{
    ifstream saveFile("savegame.txt");
    int savedScore = 0;
    int savedLives = 0;
    int savedBricks[BRICK_ROWS][BRICK_COLS];

    if (!saveFile ||
        !(saveFile >> savedScore >> savedLives) ||
        savedScore < 0 || savedLives < 1)
    {
        return false;
    }

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            if (!(saveFile >> savedBricks[r][c]) ||
                savedBricks[r][c] < 0 || savedBricks[r][c] > 3)
            {
                return false;
            }
        }
    }

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            bricks[r][c] = savedBricks[r][c];
        }
    }

    score = savedScore;
    lives = savedLives;
    return true;
}

// Saves the current score, lives, and remaining bricks for Continue.
bool saveGame(
    int bricks[BRICK_ROWS][BRICK_COLS],
    int rows, int cols, int score, int lives
)
{
    ofstream saveFile("savegame.txt");

    if (!saveFile)
    {
        return false;
    }

    saveFile << score << " " << lives << endl;

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            saveFile << bricks[r][c] << " ";
        }

        saveFile << endl;
    }

    saveFile.close();
    return static_cast<bool>(saveFile);
}

// Moves the paddle left or right while keeping it inside the game window.
void movePaddle(float& paddleX)
{
    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) &&
        paddleX <= WINDOW_WIDTH - currentPaddleWidth - PADDLE_MARGIN)
    {
        paddleX += PADDLE_SPEED;
    }

    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) &&
        paddleX >= PADDLE_MARGIN)
    {
        paddleX -= PADDLE_SPEED;
    }
}

// Draws each remaining brick using its health as the colour index.
void drawBricks(
    sf::RenderWindow& window,
    int bricks[BRICK_ROWS][BRICK_COLS],
    int rows, int cols,
    sf::Color colors[4],
    sf::RectangleShape& brick
)
{
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            if (bricks[r][c] != 0)
            {
                brick.setFillColor(colors[bricks[r][c]]);
                brick.setPosition({
                    c * BRICK_WIDTH + 1,
                    BRICKS_Y + r * BRICK_HEIGHT + 1
                    });
                window.draw(brick);
            }
        }
    }
}

// Loads the starting level and reports the specific file or format error.
bool loadInitialLevel(
    int bricks[BRICK_ROWS][BRICK_COLS], int& rows, int& cols
)
{
    ifstream levelFile("level.txt");

    if (!levelFile)
    {
        cerr << "Error: Could not open level.txt!" << endl;
        return false;
    }

    if (!(levelFile >> rows >> cols))
    {
        cerr << "Error: Invalid level dimensions!" << endl;
        return false;
    }

    if (rows != BRICK_ROWS || cols != BRICK_COLS)
    {
        cerr << "Error: Level must be 8 rows and 10 columns!" << endl;
        return false;
    }

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            if (!(levelFile >> bricks[r][c]))
            {
                cerr << "Error: Incomplete or invalid level data!" << endl;
                return false;
            }
        }
    }

    return true;
}

// Applies the common size, colour, and position settings to a text object.
void styleText(
    sf::Text& text,
    unsigned int size,
    sf::Color color,
    sf::Vector2f position
)
{
    text.setCharacterSize(size);
    text.setFillColor(color);
    text.setPosition(position);
}

void input(
    sf::RenderWindow& window,
    bool& ballLaunched,
    float& dx,
    float& dy,
    float& paddleX,
    float& ballX,
    float& ballY,
    int bricks[BRICK_ROWS][BRICK_COLS],
    int rows,
    int cols,
    int& score,
    int& lives,
    bool& gameEnded,
    int& gameState,
    sf::Text& winText,
    sf::Text& gameOverText,
    sf::Text& menuMessageText
)
{
    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            window.close();
        }
        else if (const auto* key = event->getIf<sf::Event::KeyPressed>())
        {
            switch (gameState)
            {
            case MENU:
                if (key->code == sf::Keyboard::Key::Down)
                {
                    if (menuSelection < 2)
                    {
                        menuSelection++;
                    }
                }
                else if (key->code == sf::Keyboard::Key::Up)
                {
                    if (menuSelection > 0)
                    {
                        menuSelection--;
                    }
                }

                else if (key->code == sf::Keyboard::Key::Enter)
                {
                    if (menuSelection == 0)
                    {
                        bool levelLoaded = loadLevel(bricks, rows, cols);

                        if (levelLoaded)
                        {
                            score = 0;
                            lives = INITIAL_LIVES;

                            paddleX = PADDLE_START_X;
                           
                            ballY = BALL_START_Y;

                            dx = 0;
                            dy = 0;

                            ballLaunched = false;
                            gameEnded = false;

                            // Reset power-up settings for a fresh game
                            currentPaddleWidth = PADDLE_WIDTH;
                            widePaddleTimer = 0;
                            powerUpActive = false;
                            powerUpType = ' ';
                            powerUpX = 0.f;
                            powerUpY = 0.f;

                            ballX = paddleX + (currentPaddleWidth - BALL_SIZE) / 2;

                            winText.setString("");
                            gameOverText.setString("");
                            menuMessageText.setString("");

                            gameState = PLAYING;
                        }
                        else
                        {
                            cerr << "Error: Could not reload level.txt!" << endl;
                        }
                    }
                    else if (menuSelection == 1)
                    {
                        bool saveLoaded = loadSavedGame(
                            bricks, rows, cols, score, lives
                        );

                        if (saveLoaded)
                        {
                            paddleX = PADDLE_START_X;
                           
                            ballY = BALL_START_Y;

                            dx = 0.f;
                            dy = 0.f;

                            ballLaunched = false;
                            gameEnded = false;

                            // Reset power-up settings when continuing
                            currentPaddleWidth = PADDLE_WIDTH;
                            widePaddleTimer = 0;
                            powerUpActive = false;
                            powerUpType = ' ';
                            powerUpX = 0.f;
                            powerUpY = 0.f;

                            ballX = paddleX + (currentPaddleWidth - BALL_SIZE) / 2;

                            winText.setString("");
                            gameOverText.setString("");
                            menuMessageText.setString("");

                            gameState = PLAYING;
                        }
                        else
                        {
                            menuMessageText.setString("No saved game");
                        }
                    }
                    else if (menuSelection == 2)
                    {
                        window.close();
                    }
                }
                break;
            case PLAYING:
                if (key->code == sf::Keyboard::Key::Escape)
                {
                    if (saveGame(bricks, rows, cols, score, lives))
                    {
                        gameState = MENU;
                        menuSelection = 0;
                    }
                    else
                    {
                        cerr << "Error: Could not save game!" << endl;
                    }
                }
                else if (key->code == sf::Keyboard::Key::Space &&
                    ballLaunched == false)
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

                break;
            case GAME_OVER:
                if (key->code == sf::Keyboard::Key::Enter)
                {
                    gameState = MENU;
                    menuSelection = 0;
                }
                break;
            }
        }
    }
}
void update(
    float& paddleX,
    float& ballX,
    float& ballY,
    float& dx,
    float& dy,
    int bricks[BRICK_ROWS][BRICK_COLS],
    int rows,
    int cols,
    int& score,
    int& lives,
    bool& ballLaunched,
    bool& gameEnded,
    int& gameState,
    sf::Text& winText,
    sf::Text& gameOverText
)
{
    switch (gameState)
    {
    case MENU:
        break;

    case PLAYING:
    {  // Reduce the wide paddle timer
        if (widePaddleTimer > 0)
        {
            widePaddleTimer--;

            if (widePaddleTimer == 0)
            {
                currentPaddleWidth = PADDLE_WIDTH;
            }
        }

        // Move and catch power-ups
        if (powerUpActive == true)
        {
            powerUpY += POWERUP_FALL_SPEED;

            if (powerUpY + POWERUP_SIZE >= PADDLE_Y &&
                powerUpY <= PADDLE_Y + PADDLE_HEIGHT &&
                powerUpX + POWERUP_SIZE >= paddleX &&
                powerUpX <= paddleX + currentPaddleWidth)
            {
                if (powerUpType == 'W')
                {
                    currentPaddleWidth = WIDE_PADDLE_WIDTH;
                    widePaddleTimer = WIDE_PADDLE_DURATION;

                    if (paddleX > WINDOW_WIDTH - currentPaddleWidth)
                    {
                        paddleX = WINDOW_WIDTH - currentPaddleWidth;
                    }
                }
                else if (powerUpType == '+')
                {
                    lives++;
                }

                powerUpActive = false;
            }
            else if (powerUpY > WINDOW_HEIGHT)
            {
                powerUpActive = false;
            }
        }

        if (gameEnded == false)
        {
            movePaddle(paddleX);
        }

        if (ballLaunched == false)
        {
            ballX = paddleX + (currentPaddleWidth - BALL_SIZE) / 2;
        }

        if (ballLaunched == true && gameEnded == false)
        {
            bool alreadyHit[8][10] = {};

            ballX = ballX + dx;

            if (ballX < 0 || ballX + BALL_SIZE > WINDOW_WIDTH)
            {
                ballX = ballX - dx;
                dx = -dx;
            }

            float cornerX[4] = {
                ballX, ballX + BALL_SIZE, ballX, ballX + BALL_SIZE
            };

            float cornerY[4] = {
                ballY, ballY, ballY + BALL_SIZE, ballY + BALL_SIZE
            };

            bool brickTouched = false;

            for (int i = 0; i < 4; i++)
            {
                if (cornerY[i] >= BRICKS_Y &&
                    cornerY[i] < BRICK_AREA_BOTTOM &&
                    cornerX[i] >= 0 &&
                    cornerX[i] < WINDOW_WIDTH)
                {
                    int column = cornerX[i] / BRICK_WIDTH;
                    int row = (cornerY[i] - BRICKS_Y) / BRICK_HEIGHT;

                    if (bricks[row][column] > 0)
                    {
                        brickTouched = true;

                        if (alreadyHit[row][column] == false)
                        {
                            alreadyHit[row][column] = true;

                            bricks[row][column]--;

                            if (bricks[row][column] == 0)
                            {
                                score += BRICK_BREAK_POINTS;
                                spawnPowerUp(row, column);
                            }
                            else
                            {
                                score += BRICK_HIT_POINTS;
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
            cornerX[1] = ballX + BALL_SIZE;
            cornerX[2] = ballX;
            cornerX[3] = ballX + BALL_SIZE;

            cornerY[0] = ballY;
            cornerY[1] = ballY;
            cornerY[2] = ballY + BALL_SIZE;
            cornerY[3] = ballY + BALL_SIZE;

            brickTouched = false;

            for (int i = 0; i < 4; i++)
            {
                if (cornerY[i] >= BRICKS_Y &&
                    cornerY[i] < BRICK_AREA_BOTTOM &&
                    cornerX[i] >= 0 &&
                    cornerX[i] < WINDOW_WIDTH)
                {
                    int column = cornerX[i] / BRICK_WIDTH;
                    int row = (cornerY[i] - BRICKS_Y) / BRICK_HEIGHT;

                    if (bricks[row][column] > 0)
                    {
                        brickTouched = true;

                        if (alreadyHit[row][column] == false)
                        {
                            alreadyHit[row][column] = true;

                            bricks[row][column]--;

                            if (bricks[row][column] == 0)
                            {
                                score += BRICK_BREAK_POINTS;
                                spawnPowerUp(row, column);
                            }
                            else
                            {
                                score += BRICK_HIT_POINTS;
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

            if (ballY < BALL_TOP_LIMIT)
            {
                ballY = ballY - dy;
                dy = -dy;
            }

            if (dy > 0 &&
                ballX < paddleX + currentPaddleWidth &&
                paddleX < ballX + BALL_SIZE &&
                ballY + BALL_SIZE > 650 &&
                ballY < 664)
            {
                ballY = BALL_START_Y;
                dy = BALL_LAUNCH_SPEED_Y;

                float sectionValue =
                    (ballX + BALL_SIZE / 2 - paddleX) / (currentPaddleWidth / 4);

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
                    dx = -BALL_FAST_SPEED_X;
                }
                else if (section == 1)
                {
                    dx = -BALL_LAUNCH_SPEED_X;
                }
                else if (section == 2)
                {
                    dx = BALL_LAUNCH_SPEED_X;
                }
                else if (section == 3)
                {
                    dx = BALL_FAST_SPEED_X;
                }
            }
        }

        if (ballY > WINDOW_HEIGHT && lives > 0)
        {
            lives--;

            ballLaunched = false;
            ballX = paddleX + (currentPaddleWidth - BALL_SIZE) / 2;
            ballY = BALL_START_Y;
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
            gameState = GAME_OVER;

            ofstream clearSaveFile("savegame.txt");
            clearSaveFile.close();
        }
        else if (lives == 0)
        {
            gameOverText.setString("GAME OVER");
            gameEnded = true;
            gameState = GAME_OVER;

            ofstream clearSaveFile("savegame.txt");
            clearSaveFile.close();
        }

        break;
    }

    case GAME_OVER:
        break;
    }
}
void draw(

    sf::RenderWindow& window,
    int bricks[BRICK_ROWS][BRICK_COLS],
    int rows,
    int cols,
    sf::Color colors[4],
    sf::RectangleShape& brick,
    int score,
    sf::Text& scoreText,
    int lives,
    sf::Text& livesText,
    sf::RectangleShape& paddle,
    sf::CircleShape& ball,
    sf::RectangleShape& powerUpShape,
    sf::Text& powerUpLabel,
    float paddleX,
    float ballX,
    float ballY,
    sf::Text& winText,
    sf::Text& gameOverText,
    int gameState,
    sf::Text& newGameText,
    sf::Text& continueText,
    sf::Text& exitText,
    sf::Text& finalScoreText,
    sf::Text& pressEnterText,
    sf::Text& menuMessageText
)
{
    window.clear(sf::Color::Black);

    switch (gameState)
    {
    case MENU:
        newGameText.setFillColor(
            menuSelection == 0 ? sf::Color::Yellow : sf::Color::White
        );
        continueText.setFillColor(
            menuSelection == 1 ? sf::Color::Yellow : sf::Color::White
        );
        exitText.setFillColor(
            menuSelection == 2 ? sf::Color::Yellow : sf::Color::White
        );

        window.draw(newGameText);
        window.draw(continueText);
        window.draw(exitText);
        window.draw(menuMessageText);
        break;

    case PLAYING:
        drawBricks(window, bricks, rows, cols, colors, brick);

        paddle.setSize({ currentPaddleWidth, PADDLE_HEIGHT });

        paddle.setPosition({ paddleX, PADDLE_Y });
        ball.setPosition({ ballX, ballY });

        window.draw(paddle);

        scoreText.setString("Score: " + to_string(score));
        window.draw(scoreText);

        livesText.setString("Lives: " + to_string(lives));
        window.draw(livesText);

        // Display the falling power-up
        if (powerUpActive == true)
        {
            powerUpShape.setPosition({ powerUpX, powerUpY });

            if (powerUpType == 'W')
            {
                powerUpShape.setFillColor(sf::Color::Green);
            }
            else
            {
                powerUpShape.setFillColor(sf::Color::Yellow);
            }

            powerUpLabel.setString(string(1, powerUpType));
            powerUpLabel.setPosition({ powerUpX + 6.f, powerUpY });

            window.draw(powerUpShape);
            window.draw(powerUpLabel);
        }

        window.draw(ball);
        break;

    case GAME_OVER:
        window.draw(winText);
        window.draw(gameOverText);

        finalScoreText.setString("Final Score: " + to_string(score));
        window.draw(finalScoreText);

        window.draw(pressEnterText);
        break;
    }


    window.display();
}


int main()
{
    srand(static_cast<unsigned int>(time(0)));
    sf::RenderWindow window(
        sf::VideoMode({ WINDOW_WIDTH, WINDOW_HEIGHT }),
        "Block Buster"
    );
    window.setFramerateLimit(FRAME_LIMIT);
    sf::RectangleShape paddle;
    sf::CircleShape ball(BALL_SIZE / 2);
    sf::Font font;

    if (!font.openFromFile("arial.ttf"))
    {
        cerr << "Error: Could not load font!" << endl;
        return 1;
    }
    sf::RectangleShape powerUpShape({ POWERUP_SIZE, POWERUP_SIZE });
    sf::Text powerUpLabel(font);
    powerUpLabel.setCharacterSize(18);
    powerUpLabel.setFillColor(sf::Color::Black);
    int rows = 0;
    int cols = 0;
    int bricks[BRICK_ROWS][BRICK_COLS];

    if (!loadInitialLevel(bricks, rows, cols))
    {
        return 1;
    }

    sf::Color colors[4] = {
    sf::Color(25, 25, 45),
    sf::Color::Green,
    sf::Color::Yellow,
    sf::Color::Red
    };
    sf::RectangleShape brick({ BRICK_WIDTH - 2, BRICK_HEIGHT - 2 });
    float paddleX = PADDLE_START_X;
    float ballX = paddleX + (currentPaddleWidth - BALL_SIZE) / 2;
    float ballY = BALL_START_Y;
    bool ballLaunched = false;
    float dx = 0.f;
    float dy = 0.f;
    int score = 0;
    int lives = INITIAL_LIVES;
    bool gameEnded = false;
    sf::Text scoreText(font);
    styleText(scoreText, 24, sf::Color::White, { 20.f, 10.f });
    sf::Text livesText(font);
    styleText(livesText, 24, sf::Color::White, { 200.f, 10.f });
    sf::Text winText(font);
    styleText(winText, 40, sf::Color::Yellow, { 200.f, 300.f });
    sf::Text gameOverText(font);
    styleText(gameOverText, 40, sf::Color::Red, { 180.f, 300.f });
    sf::Text newGameText(font);
    sf::Text continueText(font);
    sf::Text exitText(font);
    newGameText.setString("New Game");
    continueText.setString("Continue");
    exitText.setString("Exit");
    styleText(newGameText, 30, sf::Color::Yellow, { 220.f, 280.f });
    styleText(continueText, 30, sf::Color::White, { 220.f, 350.f });
    styleText(exitText, 30, sf::Color::White, { 220.f, 420.f });

    sf::Text finalScoreText(font);
    styleText(finalScoreText, 26, sf::Color::White, { 190.f, 370.f });

    sf::Text pressEnterText(font);
    pressEnterText.setString("Press Enter");
    styleText(pressEnterText, 24, sf::Color::White, { 220.f, 430.f });

    sf::Text menuMessageText(font);
    styleText(menuMessageText, 22, sf::Color::Yellow, { 170.f, 510.f });

    paddle.setSize({ PADDLE_WIDTH, 14.f });
    paddle.setPosition({ paddleX, PADDLE_Y });
    paddle.setFillColor(sf::Color::Green);
    while (window.isOpen())
    {
        input(
            window,
            ballLaunched,
            dx,
            dy,
            paddleX,
            ballX,
            ballY,
            bricks,
            rows,
            cols,
            score,
            lives,
            gameEnded,
            gameState,
            winText,
            gameOverText,
            menuMessageText
        );

        update(
            paddleX, ballX, ballY, dx, dy,
            bricks, rows, cols, score, lives,
            ballLaunched, gameEnded, gameState,
            winText, gameOverText
        );

        draw(
            window,
            bricks,
            rows,
            cols,
            colors,
            brick,
            score,
            scoreText,
            lives,
            livesText,
            paddle,
            ball,
            powerUpShape,
            powerUpLabel,
            paddleX,
            ballX,
            ballY,
            winText,
            gameOverText,
            gameState,
            newGameText,
            continueText,
            exitText,
            finalScoreText,
            pressEnterText,
            menuMessageText
        );
    }

    return 0;
}
