#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <ctime>
#include <cstring>
using namespace std;

const int windowLength = 1000;
const int windowWidth = 998;
const float tileSize = windowWidth / 8.0f;

const int COLOR_WHITE = 0;
const int COLOR_BLACK = 1;
const int STATE_PLAYING = 0;
const int STATE_WHITE_WON = 1;
const int STATE_BLACK_WON = 2;
const int STATE_STALEMATE = 3;

const int MAX_MOVES = 100;  // Max moves for history
const int NAME_LENGTH = 50; // Max length for player names

class ChessBoard
{
    string image;
    sf::Texture texture;
    sf::Sprite sprite;

public:
    ChessBoard(string i) : image(i)
    {
        if (!texture.loadFromFile(image))
        {
            cerr << "Error: Failed to load board texture: " << image << endl;
            throw runtime_error("Cannot load chess board texture");
        }
        sprite.setTexture(texture);
        sprite.setPosition(0, 0);
    }

    void draw(sf::RenderWindow &window)
    {
        window.draw(sprite);
    }

    ~ChessBoard() {}
};

class ChessPiece
{
protected:
    float posX, posY;
    string image;
    sf::Texture texture;
    sf::Sprite sprite;
    int color;
    bool hasMoved;
    int boardX, boardY;

public:
    ChessPiece(float x, float y, string i, int c) : posX(x), posY(y), image(i), color(c), hasMoved(false)
    {
        boardX = (int)(x / tileSize);
        boardY = (int)(y / tileSize);

        if (!texture.loadFromFile(image))
        {
            cerr << "Error: Failed to load texture: " << image << endl;
        }
        sprite.setTexture(texture);
        sprite.setPosition(posX + tileSize / 4, posY + tileSize / 4);
        float scaleX = tileSize / texture.getSize().x / 2;
        float scaleY = tileSize / texture.getSize().y / 2;
        sprite.setScale(scaleX, scaleY);
    }

    virtual void draw(sf::RenderWindow &window)
    {
        window.draw(sprite);
    }

    void setPosition(float x, float y)
    {
        posX = x;
        posY = y;
        boardX = (int)(x / tileSize);
        boardY = (int)(y / tileSize);
        sprite.setPosition(posX + tileSize / 4, posY + tileSize / 4);
        hasMoved = true;
    }

    void setHasMoved(bool moved) { hasMoved = moved; }
    sf::Sprite &getSprite() { return sprite; }
    bool contains(float x, float y) { return sprite.getGlobalBounds().contains(x, y); }
    int getColor() const { return color; }
    int getBoardX() const { return boardX; }
    int getBoardY() const { return boardY; }
    bool getHasMoved() const { return hasMoved; }

    virtual bool isValidMove(int toX, int toY, ChessPiece *board[8][8]) = 0;
    virtual ~ChessPiece() {}
};

class Pawn : public ChessPiece
{
public:
    Pawn(float x, float y, string i, int c) : ChessPiece(x, y, i, c) {}

    bool isValidMove(int toX, int toY, ChessPiece *board[8][8]) override
    {
        int direction = (color == COLOR_WHITE) ? -1 : 1;
        int startRow = (color == COLOR_WHITE) ? 6 : 1;
        int enPassantRow = (color == COLOR_WHITE) ? 3 : 4;

        if (toX == boardX && toY == boardY + direction && !board[toX][toY])
        {
            return true;
        }

        if (toX == boardX && toY == boardY + 2 * direction &&
            !board[toX][toY] && !board[toX][boardY + direction] &&
            boardY == startRow && !hasMoved)
        {
            return true;
        }

        if ((toX == boardX + 1 || toX == boardX - 1) &&
            toY == boardY + direction &&
            board[toX][toY] && board[toX][toY]->getColor() != color)
        {
            return true;
        }

        if ((toX == boardX + 1 || toX == boardX - 1) &&
            toY == boardY + direction &&
            !board[toX][toY] && boardY == enPassantRow)
        {
            ChessPiece *adjacent = board[toX][boardY];
            if (adjacent && adjacent->getColor() != color &&
                dynamic_cast<Pawn *>(adjacent))
            {
                return true;
            }
        }

        return false;
    }
};

class Rook : public ChessPiece
{
public:
    Rook(float x, float y, string i, int c) : ChessPiece(x, y, i, c) {}

    bool isValidMove(int toX, int toY, ChessPiece *board[8][8]) override
    {
        if (toX == boardX && toY == boardY)
            return false;
        if (toX != boardX && toY != boardY)
            return false;

        if (toX == boardX)
        {
            int step = (toY > boardY) ? 1 : -1;
            for (int y = boardY + step; y != toY; y += step)
            {
                if (board[toX][y])
                    return false;
            }
        }
        else
        {
            int step = (toX > boardX) ? 1 : -1;
            for (int x = boardX + step; x != toX; x += step)
            {
                if (board[x][toY])
                    return false;
            }
        }

        if (!board[toX][toY] || board[toX][toY]->getColor() != color)
        {
            return true;
        }

        return false;
    }
};

class Knight : public ChessPiece
{
public:
    Knight(float x, float y, string i, int c) : ChessPiece(x, y, i, c) {}

    bool isValidMove(int toX, int toY, ChessPiece *board[8][8]) override
    {
        int dx = abs(toX - boardX);
        int dy = abs(toY - boardY);

        if ((dx == 2 && dy == 1) || (dx == 1 && dy == 2))
        {
            if (!board[toX][toY] || board[toX][toY]->getColor() != color)
            {
                return true;
            }
        }

        return false;
    }
};

class Bishop : public ChessPiece
{
public:
    Bishop(float x, float y, string i, int c) : ChessPiece(x, y, i, c) {}

    bool isValidMove(int toX, int toY, ChessPiece *board[8][8]) override
    {
        if (toX == boardX && toY == boardY)
            return false;
        if (abs(toX - boardX) != abs(toY - boardY))
            return false;

        int stepX = (toX > boardX) ? 1 : -1;
        int stepY = (toY > boardY) ? 1 : -1;

        int x = boardX + stepX;
        int y = boardY + stepY;
        while (x != toX && y != toY)
        {
            if (board[x][y])
                return false;
            x += stepX;
            y += stepY;
        }

        if (!board[toX][toY] || board[toX][toY]->getColor() != color)
        {
            return true;
        }

        return false;
    }
};

class Queen : public ChessPiece
{
public:
    Queen(float x, float y, string i, int c) : ChessPiece(x, y, i, c) {}

    bool isValidMove(int toX, int toY, ChessPiece *board[8][8]) override
    {
        if (toX == boardX && toY == boardY)
            return false;
        if (toX != boardX && toY != boardY &&
            abs(toX - boardX) != abs(toY - boardY))
        {
            return false;
        }

        if (toX == boardX)
        {
            int step = (toY > boardY) ? 1 : -1;
            for (int y = boardY + step; y != toY; y += step)
            {
                if (board[toX][y])
                    return false;
            }
        }
        else if (toY == boardY)
        {
            int step = (toX > boardX) ? 1 : -1;
            for (int x = boardX + step; x != toX; x += step)
            {
                if (board[x][toY])
                    return false;
            }
        }
        else
        {
            int stepX = (toX > boardX) ? 1 : -1;
            int stepY = (toY > boardY) ? 1 : -1;

            int x = boardX + stepX;
            int y = boardY + stepY;
            while (x != toX && y != toY)
            {
                if (board[x][y])
                    return false;
                x += stepX;
                y += stepY;
            }
        }

        if (!board[toX][toY] || board[toX][toY]->getColor() != color)
        {
            return true;
        }

        return false;
    }
};

class King : public ChessPiece
{
public:
    King(float x, float y, string i, int c) : ChessPiece(x, y, i, c) {}

    bool isValidMove(int toX, int toY, ChessPiece *board[8][8]) override
    {
        int dx = abs(toX - boardX);
        int dy = abs(toY - boardY);

        if (dx <= 1 && dy <= 1 && (dx != 0 || dy != 0))
        {
            if (!board[toX][toY] || board[toX][toY]->getColor() != color)
            {
                return true;
            }
        }

        if (!hasMoved && dy == 0 && abs(toX - boardX) == 2)
        {
            if (toX == boardX + 2 && !board[5][boardY] && !board[6][boardY] &&
                board[7][boardY] && dynamic_cast<Rook *>(board[7][boardY]) &&
                !board[7][boardY]->getHasMoved())
            {
                return true;
            }
            if (toX == boardX - 2 && !board[3][boardY] && !board[2][boardY] &&
                !board[1][boardY] && board[0][boardY] &&
                dynamic_cast<Rook *>(board[0][boardY]) &&
                !board[0][boardY]->getHasMoved())
            {
                return true;
            }
        }

        return false;
    }
};

class ChessGame
{
    sf::RenderWindow window;
    ChessBoard *board;
    ChessPiece *pieceBoard[8][8];

    ChessPiece *pieces[32];
    int pieceCount;

    ChessPiece *selectedPiece;
    sf::Vector2f offset;

    int currentTurn;
    int gameState;

    sf::RectangleShape highlight;
    sf::CircleShape moveIndicator;

    ChessPiece *lastDoubleMovedPawn;
    int lastMoveTurn;

    // Time control
    bool useTime;
    float whiteTime;
    float blackTime;
    sf::Clock gameClock;
    sf::Font font;
    sf::Text whiteTimerText;
    sf::Text blackTimerText;
    bool fontLoaded;

    // Move history (raw pointers, no struct)
    int *moveHistory; // Array: [piece_idx, fromX, fromY, toX, toY, captured_idx, isEnPassant, isCastling, rookFromX, rookToX, pieceHasMoved, rookHasMoved, promoted_idx, lastDoubleMovedPawn_idx, lastMoveTurn]
    int moveCount;
    int moveCapacity;

    // Player names
    char *whitePlayerName;
    char *blackPlayerName;

public:
    ChessGame(bool timed = false) : pieceCount(0), selectedPiece(nullptr), currentTurn(COLOR_WHITE),
                                    gameState(STATE_PLAYING), lastDoubleMovedPawn(nullptr), lastMoveTurn(0),
                                    useTime(timed), whiteTime(600.0f), blackTime(600.0f), moveCount(0),
                                    moveCapacity(MAX_MOVES), fontLoaded(false)
    {
        window.create(sf::VideoMode(windowLength, windowWidth), "Chess Game");
        if (!window.isOpen())
        {
            cerr << "Error: Failed to create SFML window" << endl;
            throw runtime_error("Window creation failed");
        }

        try
        {
            board = new ChessBoard("../textures/chess_board.png");
        }
        catch (const runtime_error &e)
        {
            cerr << e.what() << endl;
            window.close();
            throw;
        }

        highlight.setSize(sf::Vector2f(tileSize, tileSize));
        highlight.setFillColor(sf::Color(255, 255, 0, 100));
        moveIndicator.setRadius(tileSize / 6);
        moveIndicator.setFillColor(sf::Color(0, 255, 0, 100));

        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                pieceBoard[i][j] = nullptr;
            }
        }
        for (int i = 0; i < 32; i++)
        {
            pieces[i] = nullptr;
        }

        moveHistory = new int[moveCapacity * 15];
        if (!moveHistory)
        {
            cerr << "Error: Failed to allocate moveHistory" << endl;
            window.close();
            throw runtime_error("Memory allocation failed for moveHistory");
        }
        for (int i = 0; i < moveCapacity * 15; i++)
        {
            moveHistory[i] = -1;
        }

        whitePlayerName = new char[NAME_LENGTH];
        blackPlayerName = new char[NAME_LENGTH];
        if (!whitePlayerName || !blackPlayerName)
        {
            cerr << "Error: Failed to allocate memory for player names" << endl;
            window.close();
            throw runtime_error("Memory allocation failed for player names");
        }
        for (int i = 0; i < NAME_LENGTH; i++)
        {
            whitePlayerName[i] = '\0';
            blackPlayerName[i] = '\0';
        }

        cout << "Enter White player's name: ";
        cin.getline(whitePlayerName, NAME_LENGTH);
        cout << "Enter Black player's name: ";
        cin.getline(blackPlayerName, NAME_LENGTH);

        if (useTime && font.loadFromFile("../fonts/arial.ttf"))
        {
            fontLoaded = true;
            whiteTimerText.setFont(font);
            whiteTimerText.setCharacterSize(30);
            whiteTimerText.setFillColor(sf::Color(50, 50, 50)); // Dark gray
            whiteTimerText.setPosition(10, windowWidth - 40);

            blackTimerText.setFont(font);
            blackTimerText.setCharacterSize(30);
            blackTimerText.setFillColor(sf::Color(50, 50, 50)); // Dark gray
            blackTimerText.setPosition(10, 10);
        }
        else if (useTime)
        {
            cerr << "Warning: Failed to load font ../fonts/arial.ttf. Timers will not be displayed." << endl;
        }

        initializePieces();
    }

    void initializePieces()
    {
        for (int i = 0; i < 8; i++)
        {
            pieces[pieceCount] = new Pawn(i * tileSize, 6 * tileSize, "../textures/white_pawn.png", COLOR_WHITE);
            pieceBoard[i][6] = pieces[pieceCount++];
            pieces[pieceCount] = new Pawn(i * tileSize, 1 * tileSize, "../textures/black_pawn.png", COLOR_BLACK);
            pieceBoard[i][1] = pieces[pieceCount++];
        }
        pieces[pieceCount] = new Rook(0 * tileSize, 7 * tileSize, "../textures/white_rook.png", COLOR_WHITE);
        pieceBoard[0][7] = pieces[pieceCount++];
        pieces[pieceCount] = new Rook(7 * tileSize, 7 * tileSize, "../textures/white_rook.png", COLOR_WHITE);
        pieceBoard[7][7] = pieces[pieceCount++];
        pieces[pieceCount] = new Rook(0 * tileSize, 0 * tileSize, "../textures/black_rook.png", COLOR_BLACK);
        pieceBoard[0][0] = pieces[pieceCount++];
        pieces[pieceCount] = new Rook(7 * tileSize, 0 * tileSize, "../textures/black_rook.png", COLOR_BLACK);
        pieceBoard[7][0] = pieces[pieceCount++];
        pieces[pieceCount] = new Knight(1 * tileSize, 7 * tileSize, "../textures/white_knight.png", COLOR_WHITE);
        pieceBoard[1][7] = pieces[pieceCount++];
        pieces[pieceCount] = new Knight(6 * tileSize, 7 * tileSize, "../textures/white_knight.png", COLOR_WHITE);
        pieceBoard[6][7] = pieces[pieceCount++];
        pieces[pieceCount] = new Knight(1 * tileSize, 0 * tileSize, "../textures/black_knight.png", COLOR_BLACK);
        pieceBoard[1][0] = pieces[pieceCount++];
        pieces[pieceCount] = new Knight(6 * tileSize, 0 * tileSize, "../textures/black_knight.png", COLOR_BLACK);
        pieceBoard[6][0] = pieces[pieceCount++];
        pieces[pieceCount] = new Bishop(2 * tileSize, 7 * tileSize, "../textures/white_bishop.png", COLOR_WHITE);
        pieceBoard[2][7] = pieces[pieceCount++];
        pieces[pieceCount] = new Bishop(5 * tileSize, 7 * tileSize, "../textures/white_bishop.png", COLOR_WHITE);
        pieceBoard[5][7] = pieces[pieceCount++];
        pieces[pieceCount] = new Bishop(2 * tileSize, 0 * tileSize, "../textures/black_bishop.png", COLOR_BLACK);
        pieceBoard[2][0] = pieces[pieceCount++];
        pieces[pieceCount] = new Bishop(5 * tileSize, 0 * tileSize, "../textures/black_bishop.png", COLOR_BLACK);
        pieceBoard[5][0] = pieces[pieceCount++];
        pieces[pieceCount] = new Queen(3 * tileSize, 7 * tileSize, "../textures/white_queen.png", COLOR_WHITE);
        pieceBoard[3][7] = pieces[pieceCount++];
        pieces[pieceCount] = new Queen(3 * tileSize, 0 * tileSize, "../textures/black_queen.png", COLOR_BLACK);
        pieceBoard[3][0] = pieces[pieceCount++];
        pieces[pieceCount] = new King(4 * tileSize, 7 * tileSize, "../textures/white_king.png", COLOR_WHITE);
        pieceBoard[4][7] = pieces[pieceCount++];
        pieces[pieceCount] = new King(4 * tileSize, 0 * tileSize, "../textures/black_king.png", COLOR_BLACK);
        pieceBoard[4][0] = pieces[pieceCount++];
    }

    void run()
    {
        while (window.isOpen())
        {
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed ||
                    (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
                {
                    window.close();
                }

                if (gameState == STATE_PLAYING)
                {
                    handleGameEvents(event);
                }
            }

            if (useTime && gameState == STATE_PLAYING)
            {
                float deltaTime = gameClock.restart().asSeconds();
                if (currentTurn == COLOR_WHITE)
                {
                    whiteTime -= deltaTime;
                    if (whiteTime <= 0)
                    {
                        gameState = STATE_BLACK_WON;
                        saveGameRecord();
                    }
                }
                else
                {
                    blackTime -= deltaTime;
                    if (blackTime <= 0)
                    {
                        gameState = STATE_WHITE_WON;
                        saveGameRecord();
                    }
                }
            }

            window.clear();
            drawGame();
            window.display();
        }
    }

    bool isKingInCheck(int playerColor)
    {
        int kingX = -1, kingY = -1;
        for (int i = 0; i < pieceCount; i++)
        {
            if (pieces[i] && dynamic_cast<King *>(pieces[i]) && pieces[i]->getColor() == playerColor)
            {
                kingX = pieces[i]->getBoardX();
                kingY = pieces[i]->getBoardY();
                break;
            }
        }
        if (kingX == -1)
            return false;

        for (int i = 0; i < pieceCount; i++)
        {
            if (pieces[i] && pieces[i]->getColor() != playerColor)
            {
                if (pieces[i]->isValidMove(kingX, kingY, pieceBoard))
                {
                    return true;
                }
            }
        }
        return false;
    }

    bool wouldKingBeInCheck(ChessPiece *piece, int fromX, int fromY, int toX, int toY, bool isEnPassant = false, bool isCastling = false)
    {
        ChessPiece *tempBoard[8][8];
        for (int i = 0; i < 8; i++)
            for (int j = 0; j < 8; j++)
                tempBoard[i][j] = pieceBoard[i][j];

        ChessPiece *captured = nullptr;
        if (isEnPassant)
        {
            captured = tempBoard[toX][fromY];
            tempBoard[toX][fromY] = nullptr;
        }
        else if (tempBoard[toX][toY])
        {
            captured = tempBoard[toX][toY];
            tempBoard[toX][toY] = nullptr;
        }

        if (isCastling)
        {
            int rookFromX = (toX > fromX) ? 7 : 0;
            int rookToX = (toX > fromX) ? 5 : 3;
            ChessPiece *rook = tempBoard[rookFromX][fromY];
            tempBoard[rookFromX][fromY] = nullptr;
            tempBoard[rookToX][fromY] = rook;
        }

        tempBoard[fromX][fromY] = nullptr;
        tempBoard[toX][toY] = piece;

        bool inCheck = isKingInCheck(piece->getColor());

        tempBoard[fromX][fromY] = piece;
        tempBoard[toX][toY] = isEnPassant ? nullptr : captured;
        if (isEnPassant)
            tempBoard[toX][fromY] = captured;
        if (isCastling)
        {
            int rookFromX = (toX > fromX) ? 7 : 0;
            int rookToX = (toX > fromX) ? 5 : 3;
            ChessPiece *rook = tempBoard[rookToX][fromY];
            tempBoard[rookToX][fromY] = nullptr;
            tempBoard[rookFromX][fromY] = rook;
        }

        return inCheck;
    }

    bool isValidEnPassant(ChessPiece *pawn, int toX, int toY)
    {
        if (!dynamic_cast<Pawn *>(pawn))
            return false;

        int direction = (pawn->getColor() == COLOR_WHITE) ? -1 : 1;
        if (toY != pawn->getBoardY() + direction)
            return false;

        if (abs(toX - pawn->getBoardX()) == 1 && !pieceBoard[toX][toY])
        {
            ChessPiece *target = pieceBoard[toX][pawn->getBoardY()];
            if (target == lastDoubleMovedPawn &&
                dynamic_cast<Pawn *>(target) &&
                target->getColor() != pawn->getColor() &&
                lastMoveTurn == (currentTurn == COLOR_WHITE ? COLOR_BLACK : COLOR_WHITE))
            {
                return true;
            }
        }
        return false;
    }

    void undoMove()
    {
        if (moveCount == 0)
            return;

        moveCount--;
        int idx = moveCount * 15;

        int pieceIdx = moveHistory[idx];
        int fromX = moveHistory[idx + 1];
        int fromY = moveHistory[idx + 2];
        int toX = moveHistory[idx + 3];
        int toY = moveHistory[idx + 4];
        int capturedIdx = moveHistory[idx + 5];
        bool isEnPassant = moveHistory[idx + 6] == 1;
        bool isCastling = moveHistory[idx + 7] == 1;
        int rookFromX = moveHistory[idx + 8];
        int rookToX = moveHistory[idx + 9];
        bool pieceHasMoved = moveHistory[idx + 10] == 1;
        bool rookHasMoved = moveHistory[idx + 11] == 1;
        int promotedIdx = moveHistory[idx + 12];
        int lastDoubleMovedPawnIdx = moveHistory[idx + 13];
        int lastMoveTurnBefore = moveHistory[idx + 14];

        ChessPiece *piece = pieces[pieceIdx];
        pieceBoard[toX][toY] = nullptr;
        pieceBoard[fromX][fromY] = piece;
        piece->setPosition(fromX * tileSize, fromY * tileSize);
        piece->setHasMoved(pieceHasMoved);

        if (capturedIdx != -1)
        {
            for (int i = 0; i < pieceCount; i++)
            {
                if (!pieces[i])
                {
                    pieces[i] = pieces[capturedIdx];
                    break;
                }
            }
            if (isEnPassant)
            {
                pieceBoard[toX][fromY] = pieces[capturedIdx];
            }
            else
            {
                pieceBoard[toX][toY] = pieces[capturedIdx];
            }
        }

        if (isCastling)
        {
            ChessPiece *rook = pieceBoard[rookToX][fromY];
            pieceBoard[rookToX][fromY] = nullptr;
            pieceBoard[rookFromX][fromY] = rook;
            rook->setPosition(rookFromX * tileSize, fromY * tileSize);
            rook->setHasMoved(rookHasMoved);
        }

        if (promotedIdx != -1)
        {
            for (int i = 0; i < pieceCount; i++)
            {
                if (pieces[i] == pieces[promotedIdx])
                {
                    delete pieces[i];
                    pieces[i] = piece;
                    pieceBoard[fromX][fromY] = piece;
                    piece->setPosition(fromX * tileSize, fromY * tileSize);
                    piece->setHasMoved(pieceHasMoved);
                    break;
                }
            }
        }

        lastDoubleMovedPawn = lastDoubleMovedPawnIdx == -1 ? nullptr : pieces[lastDoubleMovedPawnIdx];
        lastMoveTurn = lastMoveTurnBefore;

        currentTurn = (currentTurn == COLOR_WHITE) ? COLOR_BLACK : COLOR_WHITE;
        gameState = STATE_PLAYING;
    }

    void saveGameRecord()
    {
        ofstream file("game_records.txt", ios::app);
        if (file.is_open())
        {
            time_t now = time(0);
            char *dt = ctime(&now);
            dt[strlen(dt) - 1] = '\0'; // Remove newline

            file << dt << ", White: " << whitePlayerName << ", Black: " << blackPlayerName << ", Winner: ";
            if (gameState == STATE_WHITE_WON)
            {
                file << whitePlayerName;
            }
            else if (gameState == STATE_BLACK_WON)
            {
                file << blackPlayerName;
            }
            else
            {
                file << "Stalemate";
            }
            file << endl;
            file.close();
        }
        else
        {
            cerr << "Error: Failed to open game_records.txt for writing" << endl;
        }
    }

    void displayRecords()
    {
        ifstream file("game_records.txt");
        if (file.is_open())
        {
            string line;
            cout << "Game Records:" << endl;
            while (getline(file, line))
            {
                cout << line << endl;
            }
            file.close();
            cout << "Press Enter to continue..." << endl;
            cin.get();
        }
        else
        {
            cout << "No game records found." << endl;
            cout << "Press Enter to continue..." << endl;
            cin.get();
        }
    }

    void handleGameEvents(sf::Event &event)
    {
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Z &&
            sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
        {
            undoMove();
            return;
        }

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R &&
            sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
        {
            displayRecords();
            return;
        }

        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

            for (int i = 0; i < pieceCount; i++)
            {
                if (pieces[i] && pieces[i]->contains(mousePos.x, mousePos.y))
                {
                    if (pieces[i]->getColor() == currentTurn)
                    {
                        selectedPiece = pieces[i];
                        offset = mousePos - pieces[i]->getSprite().getPosition();
                        break;
                    }
                }
            }
        }

        if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
        {
            if (selectedPiece)
            {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                int col = (int)(mousePos.x / tileSize);
                int row = (int)(mousePos.y / tileSize);

                bool isEnPassant = isValidEnPassant(selectedPiece, col, row);
                bool isCastling = dynamic_cast<King *>(selectedPiece) && abs(col - selectedPiece->getBoardX()) == 2;
                bool isValid = (col >= 0 && col < 8 && row >= 0 && row < 8 &&
                                (selectedPiece->isValidMove(col, row, pieceBoard) || isEnPassant));

                // Reset position to avoid hanging
                selectedPiece->getSprite().setPosition(
                    selectedPiece->getBoardX() * tileSize + tileSize / 4,
                    selectedPiece->getBoardY() * tileSize + tileSize / 4);

                // Check if the move resolves check or is valid when not in check
                bool inCheck = isKingInCheck(currentTurn);
                bool moveAllowed = false;
                if (isValid)
                {
                    // Simulate the move to check if it resolves check
                    ChessPiece *tempBoard[8][8];
                    for (int i = 0; i < 8; i++)
                        for (int j = 0; j < 8; j++)
                            tempBoard[i][j] = pieceBoard[i][j];

                    ChessPiece *captured = nullptr;
                    if (isEnPassant)
                    {
                        captured = tempBoard[col][selectedPiece->getBoardY()];
                        tempBoard[col][selectedPiece->getBoardY()] = nullptr;
                    }
                    else if (tempBoard[col][row])
                    {
                        captured = tempBoard[col][row];
                        tempBoard[col][row] = nullptr;
                    }

                    if (isCastling)
                    {
                        int rookFromX = (col > selectedPiece->getBoardX()) ? 7 : 0;
                        int rookToX = (col > selectedPiece->getBoardX()) ? 5 : 3;
                        ChessPiece *rook = tempBoard[rookFromX][selectedPiece->getBoardY()];
                        tempBoard[rookFromX][selectedPiece->getBoardY()] = nullptr;
                        tempBoard[rookToX][selectedPiece->getBoardY()] = rook;
                    }

                    tempBoard[selectedPiece->getBoardX()][selectedPiece->getBoardY()] = nullptr;
                    tempBoard[col][row] = selectedPiece;

                    bool stillInCheck = isKingInCheck(currentTurn);

                    // Restore board
                    tempBoard[selectedPiece->getBoardX()][selectedPiece->getBoardY()] = selectedPiece;
                    tempBoard[col][row] = isEnPassant ? nullptr : captured;
                    if (isEnPassant)
                        tempBoard[col][selectedPiece->getBoardY()] = captured;
                    if (isCastling)
                    {
                        int rookFromX = (col > selectedPiece->getBoardX()) ? 7 : 0;
                        int rookToX = (col > selectedPiece->getBoardX()) ? 5 : 3;
                        ChessPiece *rook = tempBoard[rookToX][selectedPiece->getBoardY()];
                        tempBoard[rookToX][selectedPiece->getBoardY()] = nullptr;
                        tempBoard[rookFromX][selectedPiece->getBoardY()] = rook;
                    }

                    // Allow move if not in check or if it resolves check
                    moveAllowed = (!inCheck && !stillInCheck) || (inCheck && !stillInCheck);
                }

                // Prevent castling when in check or through check
                if (isCastling && isValid)
                {
                    if (inCheck)
                    {
                        isValid = false;
                        moveAllowed = false;
                    }
                    else
                    {
                        int oldX = selectedPiece->getBoardX();
                        int oldY = selectedPiece->getBoardY();
                        if (col > oldX)
                        {
                            if (wouldKingBeInCheck(selectedPiece, oldX, oldY, oldX + 1, oldY, false, false) ||
                                wouldKingBeInCheck(selectedPiece, oldX, oldY, oldX + 2, oldY, false, false))
                            {
                                isValid = false;
                                moveAllowed = false;
                            }
                        }
                        else
                        {
                            if (wouldKingBeInCheck(selectedPiece, oldX, oldY, oldX - 1, oldY, false, false) ||
                                wouldKingBeInCheck(selectedPiece, oldX, oldY, oldX - 2, oldY, false, false))
                            {
                                isValid = false;
                                moveAllowed = false;
                            }
                        }
                    }
                }

                if (moveAllowed && isValid && moveCount < moveCapacity)
                {
                    int oldX = selectedPiece->getBoardX();
                    int oldY = selectedPiece->getBoardY();
                    int pieceIdx = -1;
                    for (int i = 0; i < pieceCount; i++)
                    {
                        if (pieces[i] == selectedPiece)
                        {
                            pieceIdx = i;
                            break;
                        }
                    }
                    int capturedIdx = -1;
                    int promotedIdx = -1;
                    int lastDoubleMovedPawnIdx = lastDoubleMovedPawn ? -1 : -1;
                    for (int i = 0; i < pieceCount; i++)
                    {
                        if (lastDoubleMovedPawn && pieces[i] == lastDoubleMovedPawn)
                        {
                            lastDoubleMovedPawnIdx = i;
                            break;
                        }
                    }

                    // Store move data
                    int idx = moveCount * 15;
                    moveHistory[idx] = pieceIdx;
                    moveHistory[idx + 1] = oldX;
                    moveHistory[idx + 2] = oldY;
                    moveHistory[idx + 3] = col;
                    moveHistory[idx + 4] = row;
                    moveHistory[idx + 5] = -1; // capturedIdx, set later
                    moveHistory[idx + 6] = isEnPassant ? 1 : 0;
                    moveHistory[idx + 7] = isCastling ? 1 : 0;
                    moveHistory[idx + 8] = 0; // rookFromX, set later
                    moveHistory[idx + 9] = 0; // rookToX, set later
                    moveHistory[idx + 10] = selectedPiece->getHasMoved() ? 1 : 0;
                    moveHistory[idx + 11] = 0;  // rookHasMoved, set later
                    moveHistory[idx + 12] = -1; // promotedIdx, set later
                    moveHistory[idx + 13] = lastDoubleMovedPawnIdx;
                    moveHistory[idx + 14] = lastMoveTurn;

                    // Handle castling
                    if (isCastling)
                    {
                        int rookFromX = (col > oldX) ? 7 : 0;
                        int rookToX = (col > oldX) ? 5 : 3;
                        ChessPiece *rook = pieceBoard[rookFromX][oldY];
                        moveHistory[idx + 8] = rookFromX;
                        moveHistory[idx + 9] = rookToX;
                        moveHistory[idx + 11] = rook->getHasMoved() ? 1 : 0;
                        pieceBoard[rookFromX][oldY] = nullptr;
                        pieceBoard[rookToX][oldY] = rook;
                        rook->setPosition(rookToX * tileSize, oldY * tileSize);
                    }

                    // Handle captures
                    if (isEnPassant)
                    {
                        for (int i = 0; i < pieceCount; i++)
                        {
                            if (pieces[i] == pieceBoard[col][oldY])
                            {
                                capturedIdx = i;
                                break;
                            }
                        }
                        moveHistory[idx + 5] = capturedIdx;
                        pieceBoard[col][oldY] = nullptr;
                    }
                    else if (pieceBoard[col][row])
                    {
                        for (int i = 0; i < pieceCount; i++)
                        {
                            if (pieces[i] == pieceBoard[col][row])
                            {
                                capturedIdx = i;
                                break;
                            }
                        }
                        moveHistory[idx + 5] = capturedIdx;
                        capturePiece(col, row);
                    }

                    // Update en passant tracking
                    Pawn *pawn = dynamic_cast<Pawn *>(selectedPiece);
                    if (pawn && abs(row - oldY) == 2)
                    {
                        lastDoubleMovedPawn = selectedPiece;
                        lastMoveTurn = currentTurn;
                    }
                    else
                    {
                        lastDoubleMovedPawn = nullptr;
                    }

                    // Move piece
                    pieceBoard[oldX][oldY] = nullptr;
                    pieceBoard[col][row] = selectedPiece;
                    selectedPiece->setPosition(col * tileSize, row * tileSize);

                    // Handle promotion
                    if (pawn)
                    {
                        if ((pawn->getColor() == COLOR_WHITE && row == 0) ||
                            (pawn->getColor() == COLOR_BLACK && row == 7))
                        {
                            for (int i = 0; i < pieceCount; i++)
                            {
                                if (pieces[i] == pawn)
                                {
                                    string texturePath = (pawn->getColor() == COLOR_WHITE) ? "../textures/white_queen.png" : "../textures/black_queen.png";
                                    ChessPiece *newQueen = new Queen(col * tileSize, row * tileSize, texturePath, pawn->getColor());
                                    pieces[i] = newQueen;
                                    promotedIdx = i;
                                    moveHistory[idx + 12] = promotedIdx;
                                    pieceBoard[col][row] = newQueen;
                                    delete pawn;
                                    break;
                                }
                            }
                        }
                    }

                    moveCount++;

                    // Switch turn
                    currentTurn = (currentTurn == COLOR_WHITE) ? COLOR_BLACK : COLOR_WHITE;
                    checkGameState();
                }

                selectedPiece = nullptr;
            }
        }

        if (event.type == sf::Event::MouseMoved)
        {
            if (selectedPiece)
            {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                selectedPiece->getSprite().setPosition(mousePos - offset);
            }
        }
    }

    void capturePiece(int x, int y)
    {
        for (int i = 0; i < pieceCount; i++)
        {
            if (pieces[i] && pieces[i]->getBoardX() == x && pieces[i]->getBoardY() == y)
            {
                pieces[i] = nullptr;
                break;
            }
        }
    }

    void checkGameState()
    {
        bool whiteKingExists = false;
        bool blackKingExists = false;

        for (int i = 0; i < pieceCount; i++)
        {
            if (pieces[i] && dynamic_cast<King *>(pieces[i]))
            {
                if (pieces[i]->getColor() == COLOR_WHITE)
                {
                    whiteKingExists = true;
                }
                else
                {
                    blackKingExists = true;
                }
            }
        }

        if (!whiteKingExists)
        {
            gameState = STATE_BLACK_WON;
            saveGameRecord();
        }
        else if (!blackKingExists)
        {
            gameState = STATE_WHITE_WON;
            saveGameRecord();
        }
    }

    void drawGame()
    {
        board->draw(window);

        if (selectedPiece)
        {
            highlight.setPosition(selectedPiece->getBoardX() * tileSize,
                                  selectedPiece->getBoardY() * tileSize);
            window.draw(highlight);
        }

        for (int i = 0; i < pieceCount; i++)
        {
            if (pieces[i])
            {
                pieces[i]->draw(window);
            }
        }

        if (useTime && fontLoaded)
        {
            int whiteMinutes = static_cast<int>(whiteTime / 60);
            int whiteSeconds = static_cast<int>(whiteTime) % 60;
            int blackMinutes = static_cast<int>(blackTime / 60);
            int blackSeconds = static_cast<int>(blackTime) % 60;

            char whiteBuffer[16];
            char blackBuffer[16];
            snprintf(whiteBuffer, sizeof(whiteBuffer), "White: %02d:%02d", whiteMinutes, whiteSeconds);
            snprintf(blackBuffer, sizeof(blackBuffer), "Black: %02d:%02d", blackMinutes, blackSeconds);

            whiteTimerText.setString(whiteBuffer);
            blackTimerText.setString(blackBuffer);
            window.draw(whiteTimerText);
            window.draw(blackTimerText);
        }

        if (gameState != STATE_PLAYING && fontLoaded)
        {
            sf::Text text;
            text.setFont(font);
            text.setCharacterSize(50);
            text.setFillColor(sf::Color::Red);
            text.setStyle(sf::Text::Bold);
            text.setPosition(windowWidth / 2 - 150, windowWidth / 2 - 25);

            if (gameState == STATE_WHITE_WON)
                text.setString("White Wins!");
            else if (gameState == STATE_BLACK_WON)
                text.setString("Black Wins!");
            else if (gameState == STATE_STALEMATE)
                text.setString("Stalemate!");

            window.draw(text);
        }
    }

    ~ChessGame()
    {
        delete board;
        for (int i = 0; i < pieceCount; i++)
        {
            if (pieces[i])
            {
                delete pieces[i];
            }
        }
        delete[] moveHistory;
        delete[] whitePlayerName;
        delete[] blackPlayerName;
    }
};

int main()
{
    try
    {
        ChessGame game(true);
        game.run();
    }
    catch (const runtime_error &e)
    {
        cerr << "Fatal error: " << e.what() << endl;
        return 1;
    }
    return 0;
}