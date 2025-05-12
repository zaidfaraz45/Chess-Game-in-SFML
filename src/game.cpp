#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <ctime>
#include <cstring>
#include <vector>
using namespace std;

const int windowlength = 1000;
const int windowwidth = 998;
const float tilesize = windowwidth / 8.0f;

const int colorwhite = 0;
const int colorblack = 1;
const int stateplaying = 0;
const int statewhitewon = 1;
const int stateblackwon = 2;
const int statestalemate = 3;
const int staterecords = 4;

const int maxmoves = 100;
const int namelength = 50;

const int piecepawn = 0;
const int piecerook = 1;
const int pieceknight = 2;
const int piecebishop = 3;
const int piecequeen = 4;
const int pieceking = 5;

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
    int pieceType;

public:
    ChessPiece(float x, float y, string i, int c, int type) : posX(x), posY(y), image(i), color(c), hasMoved(false), pieceType(type)
    {
        boardX = (int)(x / tilesize);
        boardY = (int)(y / tilesize);

        if (!texture.loadFromFile(image))
        {
            throw runtime_error("Cannot load texture: " + image);
        }
        sprite.setTexture(texture);
        sprite.setPosition(posX + tilesize / 4, posY + tilesize / 4);
        float scaleX = tilesize / texture.getSize().x / 2;
        float scaleY = tilesize / texture.getSize().y / 2;
        sprite.setScale(scaleX, scaleY);
    }

    void draw(sf::RenderWindow &window)
    {
        window.draw(sprite);
    }

    void setPosition(float x, float y)
    {
        posX = x;
        posY = y;
        boardX = (int)(x / tilesize);
        boardY = (int)(y / tilesize);
        sprite.setPosition(posX + tilesize / 4, posY + tilesize / 4);
    }

    void setHasMoved(bool moved) { hasMoved = moved; }
    sf::Sprite &getSprite() { return sprite; }
    bool contains(float x, float y) { return sprite.getGlobalBounds().contains(x, y); }
    int getColor() const { return color; }
    int getBoardX() const { return boardX; }
    int getBoardY() const { return boardY; }
    bool getHasMoved() const { return hasMoved; }
    int getPieceType() const { return pieceType; }

    virtual bool isValidMove(int toX, int toY, ChessPiece *board[8][8]) = 0;
    virtual ~ChessPiece() {}
};

class Pawn : public ChessPiece
{
public:
    Pawn(float x, float y, string i, int c) : ChessPiece(x, y, i, c, piecepawn) {}

    bool isValidMove(int toX, int toY, ChessPiece *board[8][8]) override
    {
        if (!board || toX < 0 || toX >= 8 || toY < 0 || toY >= 8)
            return false;
        int direction = (color == colorwhite) ? -1 : 1;
        int startRow = (color == colorwhite) ? 6 : 1;

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

        return false;
    }
};

class Rook : public ChessPiece
{
public:
    Rook(float x, float y, string i, int c) : ChessPiece(x, y, i, c, piecerook) {}

    bool isValidMove(int toX, int toY, ChessPiece *board[8][8]) override
    {
        if (!board || toX < 0 || toX >= 8 || toY < 0 || toY >= 8)
            return false;
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
    Knight(float x, float y, string i, int c) : ChessPiece(x, y, i, c, pieceknight) {}

    bool isValidMove(int toX, int toY, ChessPiece *board[8][8]) override
    {
        if (!board || toX < 0 || toX >= 8 || toY < 0 || toY >= 8)
            return false;
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
    Bishop(float x, float y, string i, int c) : ChessPiece(x, y, i, c, piecebishop) {}

    bool isValidMove(int toX, int toY, ChessPiece *board[8][8]) override
    {
        if (!board || toX < 0 || toX >= 8 || toY < 0 || toY >= 8)
            return false;
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
    Queen(float x, float y, string i, int c) : ChessPiece(x, y, i, c, piecequeen) {}

    bool isValidMove(int toX, int toY, ChessPiece *board[8][8]) override
    {
        if (!board || toX < 0 || toX >= 8 || toY < 0 || toY >= 8)
            return false;
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
    King(float x, float y, string i, int c) : ChessPiece(x, y, i, c, pieceking) {}

    bool isValidMove(int toX, int toY, ChessPiece *board[8][8]) override
    {
        if (!board || toX < 0 || toX >= 8 || toY < 0 || toY >= 8)
            return false;
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
                board[7][boardY] && board[7][boardY]->getPieceType() == piecerook &&
                !board[7][boardY]->getHasMoved())
            {
                return true;
            }
            if (toX == boardX - 2 && !board[3][boardY] && !board[2][boardY] &&
                !board[1][boardY] && board[0][boardY] &&
                board[0][boardY]->getPieceType() == piecerook &&
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

    bool useTime;
    float whiteTime;
    float blackTime;
    sf::Clock gameClock;
    sf::Font font;
    sf::Text whiteTimerText;
    sf::Text blackTimerText;
    bool fontLoaded;

    int *moveHistory;
    int moveCount;
    int moveCapacity;

    char *whitePlayerName;
    char *blackPlayerName;

    vector<string> gameRecords;
    vector<ChessPiece *> capturedPieces;
    bool keyPressed;

public:
    ChessGame(bool timed = false) : pieceCount(0), selectedPiece(nullptr), currentTurn(colorwhite),
                                    gameState(stateplaying), lastDoubleMovedPawn(nullptr), lastMoveTurn(0),
                                    useTime(timed), whiteTime(600.0f), blackTime(600.0f), moveCount(0),
                                    moveCapacity(maxmoves), fontLoaded(false), keyPressed(false)
    {
        whitePlayerName = new char[namelength];
        blackPlayerName = new char[namelength];
        if (!whitePlayerName || !blackPlayerName)
        {
            throw runtime_error("Memory allocation failed for player names");
        }
        for (int i = 0; i < namelength; i++)
        {
            whitePlayerName[i] = '\0';
            blackPlayerName[i] = '\0';
        }

        cout << "Enter White player's name: ";
        cin.getline(whitePlayerName, namelength);
        cout << "Enter Black player's name: ";
        cin.getline(blackPlayerName, namelength);

        window.create(sf::VideoMode(windowlength, windowwidth), "Chess Game");
        if (!window.isOpen())
        {
            throw runtime_error("Window creation failed");
        }

        try
        {
            board = new ChessBoard("../textures/chess_board.png");
        }
        catch (const runtime_error &e)
        {
            window.close();
            throw;
        }

        highlight.setSize(sf::Vector2f(tilesize, tilesize));
        highlight.setFillColor(sf::Color(255, 255, 0, 100));
        moveIndicator.setRadius(tilesize / 6);
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

        moveHistory = new int[moveCapacity * 16];
        if (!moveHistory)
        {
            window.close();
            throw runtime_error("Memory allocation failed for moveHistory");
        }
        for (int i = 0; i < moveCapacity * 16; i++)
        {
            moveHistory[i] = -1;
        }

        if (useTime && font.loadFromFile("../fonts/arial.ttf"))
        {
            fontLoaded = true;
            whiteTimerText.setFont(font);
            whiteTimerText.setCharacterSize(30);
            whiteTimerText.setFillColor(sf::Color(50, 50, 50));
            whiteTimerText.setPosition(10, windowwidth - 40);

            blackTimerText.setFont(font);
            blackTimerText.setCharacterSize(30);
            blackTimerText.setFillColor(sf::Color(50, 50, 50));
            blackTimerText.setPosition(10, 10);
        }

        initializePieces();
    }

    void initializePieces()
    {
        for (int i = 0; i < 8; i++)
        {
            pieces[pieceCount] = new Pawn(i * tilesize, 6 * tilesize, "../textures/white_pawn.png", colorwhite);
            pieceBoard[i][6] = pieces[pieceCount++];
            pieces[pieceCount] = new Pawn(i * tilesize, 1 * tilesize, "../textures/black_pawn.png", colorblack);
            pieceBoard[i][1] = pieces[pieceCount++];
        }
        pieces[pieceCount] = new Rook(0 * tilesize, 7 * tilesize, "../textures/white_rook.png", colorwhite);
        pieceBoard[0][7] = pieces[pieceCount++];
        pieces[pieceCount] = new Rook(7 * tilesize, 7 * tilesize, "../textures/white_rook.png", colorwhite);
        pieceBoard[7][7] = pieces[pieceCount++];
        pieces[pieceCount] = new Rook(0 * tilesize, 0 * tilesize, "../textures/black_rook.png", colorblack);
        pieceBoard[0][0] = pieces[pieceCount++];
        pieces[pieceCount] = new Rook(7 * tilesize, 0 * tilesize, "../textures/black_rook.png", colorblack);
        pieceBoard[7][0] = pieces[pieceCount++];
        pieces[pieceCount] = new Knight(1 * tilesize, 7 * tilesize, "../textures/white_knight.png", colorwhite);
        pieceBoard[1][7] = pieces[pieceCount++];
        pieces[pieceCount] = new Knight(6 * tilesize, 7 * tilesize, "../textures/white_knight.png", colorwhite);
        pieceBoard[6][7] = pieces[pieceCount++];
        pieces[pieceCount] = new Knight(1 * tilesize, 0 * tilesize, "../textures/black_knight.png", colorblack);
        pieceBoard[1][0] = pieces[pieceCount++];
        pieces[pieceCount] = new Knight(6 * tilesize, 0 * tilesize, "../textures/black_knight.png", colorblack);
        pieceBoard[6][0] = pieces[pieceCount++];
        pieces[pieceCount] = new Bishop(2 * tilesize, 7 * tilesize, "../textures/white_bishop.png", colorwhite);
        pieceBoard[2][7] = pieces[pieceCount++];
        pieces[pieceCount] = new Bishop(5 * tilesize, 7 * tilesize, "../textures/white_bishop.png", colorwhite);
        pieceBoard[5][7] = pieces[pieceCount++];
        pieces[pieceCount] = new Bishop(2 * tilesize, 0 * tilesize, "../textures/black_bishop.png", colorblack);
        pieceBoard[2][0] = pieces[pieceCount++];
        pieces[pieceCount] = new Bishop(5 * tilesize, 0 * tilesize, "../textures/black_bishop.png", colorblack);
        pieceBoard[5][0] = pieces[pieceCount++];
        pieces[pieceCount] = new Queen(3 * tilesize, 7 * tilesize, "../textures/white_queen.png", colorwhite);
        pieceBoard[3][7] = pieces[pieceCount++];
        pieces[pieceCount] = new Queen(3 * tilesize, 0 * tilesize, "../textures/black_queen.png", colorblack);
        pieceBoard[3][0] = pieces[pieceCount++];
        pieces[pieceCount] = new King(4 * tilesize, 7 * tilesize, "../textures/white_king.png", colorwhite);
        pieceBoard[4][7] = pieces[pieceCount++];
        pieces[pieceCount] = new King(4 * tilesize, 0 * tilesize, "../textures/black_king.png", colorblack);
        pieceBoard[4][0] = pieces[pieceCount++];
    }

    void run()
    {
        if (useTime)
        {
            gameClock.restart();
        }

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

                handleGameEvents(event);
            }

            if (useTime && gameState == stateplaying)
            {
                float deltaTime = gameClock.restart().asSeconds();
                if (currentTurn == colorwhite)
                {
                    whiteTime -= deltaTime;
                    if (whiteTime <= 0)
                    {
                        gameState = stateblackwon;
                        saveGameRecord();
                    }
                }
                else
                {
                    blackTime -= deltaTime;
                    if (blackTime <= 0)
                    {
                        gameState = statewhitewon;
                        saveGameRecord();
                    }
                }
            }

            window.clear();
            drawGame();
            window.display();
        }
    }

    bool isKingInCheck(int playerColor, int kingX, int kingY, ChessPiece *board[8][8])
    {
        if (!board || kingX < 0 || kingX >= 8 || kingY < 0 || kingY >= 8)
            return false;

        for (int x = 0; x < 8; x++)
        {
            for (int y = 0; y < 8; y++)
            {
                ChessPiece *piece = board[x][y];
                if (piece && piece->getColor() != playerColor)
                {
                    if (piece->isValidMove(kingX, kingY, board))
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool findKingPosition(int playerColor, int &kingX, int &kingY, ChessPiece *board[8][8])
    {
        if (!board)
            return false;
        for (int x = 0; x < 8; x++)
        {
            for (int y = 0; y < 8; y++)
            {
                ChessPiece *piece = board[x][y];
                if (piece && piece->getPieceType() == pieceking && piece->getColor() == playerColor)
                {
                    kingX = x;
                    kingY = y;
                    return true;
                }
            }
        }
        return false;
    }

    bool wouldKingBeInCheck(ChessPiece *piece, int fromX, int fromY, int toX, int toY, bool isEnPassant = false, bool isCastling = false)
    {
        if (!piece || !pieceBoard || toX < 0 || toX >= 8 || toY < 0 || toY >= 8 || fromX < 0 || fromX >= 8 || fromY < 0 || fromY >= 8)
        {
            return false;
        }

        int playerColor = piece->getColor();
        ChessPiece *tempBoard[8][8];

        for (int i = 0; i < 8; i++)
            for (int j = 0; j < 8; j++)
                tempBoard[i][j] = pieceBoard[i][j];

        if (isEnPassant)
        {
            tempBoard[toX][fromY] = nullptr;
        }
        else if (tempBoard[toX][toY])
        {
            tempBoard[toX][toY] = nullptr;
        }

        if (isCastling)
        {
            int rookFromX = (toX > fromX) ? 7 : 0;
            int rookToX = (toX > fromX) ? 5 : 3;
            ChessPiece *rook = tempBoard[rookFromX][fromY];
            if (rook)
            {
                tempBoard[rookFromX][fromY] = nullptr;
                tempBoard[rookToX][fromY] = rook;
            }
        }

        tempBoard[fromX][fromY] = nullptr;
        tempBoard[toX][toY] = piece;

        int kingX, kingY;
        if (piece->getPieceType() == pieceking)
        {
            kingX = toX;
            kingY = toY;
        }
        else
        {
            if (!findKingPosition(playerColor, kingX, kingY, tempBoard))
            {
                return false;
            }
        }

        return isKingInCheck(playerColor, kingX, kingY, tempBoard);
    }

    bool isValidEnPassant(ChessPiece *pawn, int toX, int toY)
    {
        if (!pawn || pawn->getPieceType() != piecepawn || toX < 0 || toX >= 8 || toY < 0 || toY >= 8)
            return false;

        int direction = (pawn->getColor() == colorwhite) ? -1 : 1;
        if (toY != pawn->getBoardY() + direction)
            return false;

        if (abs(toX - pawn->getBoardX()) == 1 && !pieceBoard[toX][toY])
        {
            ChessPiece *target = pieceBoard[toX][pawn->getBoardY()];
            if (target == lastDoubleMovedPawn &&
                target->getPieceType() == piecepawn &&
                target->getColor() != pawn->getColor() &&
                lastMoveTurn == (currentTurn == colorwhite ? colorblack : colorwhite))
            {
                return true;
            }
        }
        return false;
    }

    bool hasLegalMoves(int playerColor)
    {
        for (int fromX = 0; fromX < 8; fromX++)
        {
            for (int fromY = 0; fromY < 8; fromY++)
            {
                ChessPiece *piece = pieceBoard[fromX][fromY];
                if (!piece || piece->getColor() != playerColor)
                {
                    continue;
                }
                for (int toX = 0; toX < 8; toX++)
                {
                    for (int toY = 0; toY < 8; toY++)
                    {
                        bool isEnPassant = isValidEnPassant(piece, toX, toY);
                        bool isCastling = piece->getPieceType() == pieceking &&
                                          abs(toX - fromX) == 2 && toY == fromY;
                        if ((piece->isValidMove(toX, toY, pieceBoard) || isEnPassant) &&
                            !wouldKingBeInCheck(piece, fromX, fromY, toX, toY, isEnPassant, isCastling))
                        {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    void undoMove()
    {
        if (moveCount <= 0)
            return;

        moveCount--;
        int idx = moveCount * 16;

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
        int originalPieceType = moveHistory[idx + 12];
        int promotedIdx = moveHistory[idx + 13];
        int lastDoubleMovedPawnIdx = moveHistory[idx + 14];
        int lastMoveTurnBefore = moveHistory[idx + 15];

        ChessPiece *piece = pieceIdx >= 0 && pieceIdx < 32 ? pieces[pieceIdx] : nullptr;
        if (!piece || fromX < 0 || fromX >= 8 || fromY < 0 || fromY >= 8 || toX < 0 || toX >= 8 || toY < 0 || toY >= 8)
            return;

        if (promotedIdx != -1 && originalPieceType == piecepawn)
        {
            for (int i = 0; i < 32; i++)
            {
                if (pieces[i] == piece)
                {
                    string texturePath = (piece->getColor() == colorwhite) ? "../textures/white_pawn.png" : "../textures/black_pawn.png";
                    ChessPiece *newPiece = new Pawn(fromX * tilesize, fromY * tilesize, texturePath, piece->getColor());
                    newPiece->setHasMoved(pieceHasMoved);
                    capturedPieces.push_back(pieces[i]);
                    pieces[i] = newPiece;
                    piece = newPiece;
                    break;
                }
            }
        }

        pieceBoard[toX][toY] = nullptr;
        pieceBoard[fromX][fromY] = piece;
        piece->setPosition(fromX * tilesize, fromY * tilesize);
        piece->setHasMoved(pieceHasMoved);

        if (capturedIdx >= 0 && capturedIdx < (int)capturedPieces.size())
        {
            ChessPiece *capturedPiece = capturedPieces[capturedIdx];
            if (capturedPiece)
            {
                for (int i = 0; i < 32; i++)
                {
                    if (!pieces[i])
                    {
                        pieces[i] = capturedPiece;
                        break;
                    }
                }
                if (isEnPassant)
                {
                    pieceBoard[toX][fromY] = capturedPiece;
                    capturedPiece->setPosition(toX * tilesize, fromY * tilesize);
                }
                else
                {
                    pieceBoard[toX][toY] = capturedPiece;
                    capturedPiece->setPosition(toX * tilesize, toY * tilesize);
                }
                capturedPieces[capturedIdx] = nullptr;
            }
        }

        if (isCastling)
        {
            ChessPiece *rook = pieceBoard[rookToX][fromY];
            if (rook && rookFromX >= 0 && rookFromX < 8 && rookToX >= 0 && rookToX < 8)
            {
                pieceBoard[rookToX][fromY] = nullptr;
                pieceBoard[rookFromX][fromY] = rook;
                rook->setPosition(rookFromX * tilesize, fromY * tilesize);
                rook->setHasMoved(rookHasMoved);
            }
        }

        lastDoubleMovedPawn = lastDoubleMovedPawnIdx >= 0 && lastDoubleMovedPawnIdx < 32 ? pieces[lastDoubleMovedPawnIdx] : nullptr;
        lastMoveTurn = lastMoveTurnBefore;

        currentTurn = (currentTurn == colorwhite) ? colorblack : colorwhite;
        gameState = stateplaying;
    }

    void saveGameRecord()
    {
        ofstream file("game_records.txt", ios::app);
        if (file.is_open())
        {
            time_t now = time(0);
            char *dt = ctime(&now);
            if (dt)
            {
                dt[strlen(dt) - 1] = '\0';
                file << dt << ", White: " << whitePlayerName << ", Black: " << blackPlayerName << ", Winner: ";
                if (gameState == statewhitewon)
                {
                    file << whitePlayerName;
                }
                else if (gameState == stateblackwon)
                {
                    file << blackPlayerName;
                }
                else
                {
                    file << "Stalemate";
                }
                file << endl;
            }
            file.close();
        }
    }

    void displayRecords()
    {
        gameRecords.clear();
        ifstream file("game_records.txt");
        if (file.is_open())
        {
            string line;
            while (getline(file, line))
            {
                gameRecords.push_back(line);
            }
            file.close();
        }
        else
        {
            gameRecords.push_back("No game records found.");
        }
        gameState = staterecords;
        sf::Event event;
        while (window.pollEvent(event))
        {
        }
    }

    void handleGameEvents(sf::Event &event)
    {
        if (event.type == sf::Event::KeyPressed)
        {
            if (!keyPressed && event.key.code == sf::Keyboard::Z &&
                sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && gameState == stateplaying)
            {
                undoMove();
                keyPressed = true;
            }
            if (!keyPressed && event.key.code == sf::Keyboard::R &&
                sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
            {
                if (gameState == staterecords)
                {
                    gameState = stateplaying;
                    sf::Event clearEvent;
                    while (window.pollEvent(clearEvent))
                    {
                    }
                }
                else
                {
                    displayRecords();
                }
                keyPressed = true;
            }
        }
        if (event.type == sf::Event::KeyReleased)
        {
            keyPressed = false;
        }

        if (gameState != stateplaying)
            return;

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
                int col = (int)(mousePos.x / tilesize);
                int row = (int)(mousePos.y / tilesize);

                bool isEnPassant = isValidEnPassant(selectedPiece, col, row);
                bool isCastling = selectedPiece->getPieceType() == pieceking &&
                                  abs(col - selectedPiece->getBoardX()) == 2;
                bool isValid = (col >= 0 && col < 8 && row >= 0 && row < 8 &&
                                (selectedPiece->isValidMove(col, row, pieceBoard) || isEnPassant));

                selectedPiece->getSprite().setPosition(
                    selectedPiece->getBoardX() * tilesize + tilesize / 4,
                    selectedPiece->getBoardY() * tilesize + tilesize / 4);

                int kingX = -1, kingY = -1;
                bool inCheck = findKingPosition(currentTurn, kingX, kingY, pieceBoard) &&
                               isKingInCheck(currentTurn, kingX, kingY, pieceBoard);
                bool moveAllowed = false;
                if (isValid)
                {
                    ChessPiece *tempBoard[8][8];
                    for (int i = 0; i < 8; i++)
                        for (int j = 0; j < 8; j++)
                            tempBoard[i][j] = pieceBoard[i][j];

                    if (isEnPassant)
                    {
                        tempBoard[col][selectedPiece->getBoardY()] = nullptr;
                    }
                    else if (tempBoard[col][row])
                    {
                        tempBoard[col][row] = nullptr;
                    }

                    if (isCastling)
                    {
                        int rookFromX = (col > selectedPiece->getBoardX()) ? 7 : 0;
                        int rookToX = (col > selectedPiece->getBoardX()) ? 5 : 3;
                        ChessPiece *rook = tempBoard[rookFromX][selectedPiece->getBoardY()];
                        if (rook)
                        {
                            tempBoard[rookFromX][selectedPiece->getBoardY()] = nullptr;
                            tempBoard[rookToX][selectedPiece->getBoardY()] = rook;
                        }
                    }

                    tempBoard[selectedPiece->getBoardX()][selectedPiece->getBoardY()] = nullptr;
                    tempBoard[col][row] = selectedPiece;

                    int newKingX, newKingY;
                    if (selectedPiece->getPieceType() == pieceking)
                    {
                        newKingX = col;
                        newKingY = row;
                    }
                    else
                    {
                        newKingX = kingX;
                        newKingY = kingY;
                    }

                    bool stillInCheck = isKingInCheck(currentTurn, newKingX, newKingY, tempBoard);

                    moveAllowed = (!inCheck && !stillInCheck) || (inCheck && !stillInCheck);
                }

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
                    int originalPieceType = selectedPiece->getPieceType();
                    int lastDoubleMovedPawnIdx = lastDoubleMovedPawn ? -1 : -1;
                    for (int i = 0; i < pieceCount; i++)
                    {
                        if (lastDoubleMovedPawn && pieces[i] == lastDoubleMovedPawn)
                        {
                            lastDoubleMovedPawnIdx = i;
                            break;
                        }
                    }

                    int idx = moveCount * 16;
                    moveHistory[idx] = pieceIdx;
                    moveHistory[idx + 1] = oldX;
                    moveHistory[idx + 2] = oldY;
                    moveHistory[idx + 3] = col;
                    moveHistory[idx + 4] = row;
                    moveHistory[idx + 5] = -1;
                    moveHistory[idx + 6] = isEnPassant ? 1 : 0;
                    moveHistory[idx + 7] = isCastling ? 1 : 0;
                    moveHistory[idx + 8] = 0;
                    moveHistory[idx + 9] = 0;
                    moveHistory[idx + 10] = selectedPiece->getHasMoved() ? 1 : 0;
                    moveHistory[idx + 11] = 0;
                    moveHistory[idx + 12] = originalPieceType;
                    moveHistory[idx + 13] = -1;
                    moveHistory[idx + 14] = lastDoubleMovedPawnIdx;
                    moveHistory[idx + 15] = lastMoveTurn;

                    if (isCastling)
                    {
                        int rookFromX = (col > oldX) ? 7 : 0;
                        int rookToX = (col > oldX) ? 5 : 3;
                        ChessPiece *rook = pieceBoard[rookFromX][oldY];
                        if (rook)
                        {
                            moveHistory[idx + 8] = rookFromX;
                            moveHistory[idx + 9] = rookToX;
                            moveHistory[idx + 11] = rook->getHasMoved() ? 1 : 0;
                            pieceBoard[rookFromX][oldY] = nullptr;
                            pieceBoard[rookToX][oldY] = rook;
                            rook->setPosition(rookToX * tilesize, oldY * tilesize);
                            rook->setHasMoved(true);
                        }
                    }

                    if (isEnPassant)
                    {
                        ChessPiece *captured = pieceBoard[col][oldY];
                        if (captured)
                        {
                            capturedIdx = capturedPieces.size();
                            capturedPieces.push_back(captured);
                            moveHistory[idx + 5] = capturedIdx;
                            for (int i = 0; i < pieceCount; i++)
                            {
                                if (pieces[i] == captured)
                                {
                                    pieces[i] = nullptr;
                                    break;
                                }
                            }
                            pieceBoard[col][oldY] = nullptr;
                        }
                    }
                    else if (pieceBoard[col][row])
                    {
                        ChessPiece *captured = pieceBoard[col][row];
                        capturedIdx = capturedPieces.size();
                        capturedPieces.push_back(captured);
                        moveHistory[idx + 5] = capturedIdx;
                        for (int i = 0; i < pieceCount; i++)
                        {
                            if (pieces[i] == captured)
                            {
                                pieces[i] = nullptr;
                                break;
                            }
                        }
                        pieceBoard[col][row] = nullptr;
                    }

                    if (selectedPiece->getPieceType() == piecepawn && abs(row - oldY) == 2)
                    {
                        lastDoubleMovedPawn = selectedPiece;
                        lastMoveTurn = currentTurn;
                    }
                    else
                    {
                        lastDoubleMovedPawn = nullptr;
                    }

                    pieceBoard[oldX][oldY] = nullptr;
                    pieceBoard[col][row] = selectedPiece;
                    selectedPiece->setPosition(col * tilesize, row * tilesize);
                    selectedPiece->setHasMoved(true);

                    if (selectedPiece->getPieceType() == piecepawn)
                    {
                        if ((selectedPiece->getColor() == colorwhite && row == 0) ||
                            (selectedPiece->getColor() == colorblack && row == 7))
                        {
                            for (int i = 0; i < pieceCount; i++)
                            {
                                if (pieces[i] == selectedPiece)
                                {
                                    string texturePath = (selectedPiece->getColor() == colorwhite) ? "../textures/white_queen.png" : "../textures/black_queen.png";
                                    ChessPiece *newQueen = new Queen(col * tilesize, row * tilesize, texturePath, selectedPiece->getColor());
                                    newQueen->setHasMoved(true);
                                    capturedPieces.push_back(pieces[i]);
                                    pieces[i] = newQueen;
                                    promotedIdx = i;
                                    moveHistory[idx + 13] = promotedIdx;
                                    pieceBoard[col][row] = newQueen;
                                    selectedPiece = newQueen;
                                    break;
                                }
                            }
                        }
                    }

                    moveCount++;

                    currentTurn = (currentTurn == colorwhite) ? colorblack : colorwhite;
                    checkGameState();
                }

                selectedPiece = nullptr;
            }
        }

        if (event.type == sf::Event::MouseMoved && gameState == stateplaying)
        {
            if (selectedPiece)
            {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                selectedPiece->getSprite().setPosition(mousePos - offset);
            }
        }
    }

    void checkGameState()
    {
        bool whiteKingExists = false;
        bool blackKingExists = false;

        for (int i = 0; i < pieceCount; i++)
        {
            if (pieces[i] && pieces[i]->getPieceType() == pieceking)
            {
                if (pieces[i]->getColor() == colorwhite)
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
            gameState = stateblackwon;
            saveGameRecord();
            return;
        }
        else if (!blackKingExists)
        {
            gameState = statewhitewon;
            saveGameRecord();
            return;
        }

        int kingX = -1, kingY = -1;
        if (findKingPosition(currentTurn, kingX, kingY, pieceBoard))
        {
            bool inCheck = isKingInCheck(currentTurn, kingX, kingY, pieceBoard);
            bool hasMoves = hasLegalMoves(currentTurn);

            if (inCheck && !hasMoves)
            {
                gameState = (currentTurn == colorwhite) ? stateblackwon : statewhitewon;
                saveGameRecord();
            }
            else if (!inCheck && !hasMoves)
            {
                gameState = statestalemate;
                saveGameRecord();
            }
        }
    }

    void drawGame()
    {
        board->draw(window);

        if (gameState == staterecords && fontLoaded)
        {
            sf::RectangleShape background(sf::Vector2f(windowlength, windowwidth));
            background.setFillColor(sf::Color(0, 0, 0, 200));
            window.draw(background);

            sf::Text text;
            text.setFont(font);
            text.setCharacterSize(20);
            text.setFillColor(sf::Color::White);

            float yOffset = 50;
            for (const auto &record : gameRecords)
            {
                text.setString(record);
                text.setPosition(10, yOffset);
                window.draw(text);
                yOffset += 30;
            }
            return;
        }

        if (selectedPiece)
        {
            highlight.setPosition(selectedPiece->getBoardX() * tilesize,
                                  selectedPiece->getBoardY() * tilesize);
            window.draw(highlight);
        }

        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                if (pieceBoard[i][j])
                {
                    pieceBoard[i][j]->draw(window);
                }
            }
        }

        if (useTime && fontLoaded && gameState == stateplaying)
        {
            int whiteMinutes = (int)(whiteTime / 60);
            int whiteSeconds = (int)whiteTime % 60;
            int blackMinutes = (int)(blackTime / 60);
            int blackSeconds = (int)blackTime % 60;

            char whiteBuffer[16];
            char blackBuffer[16];
            snprintf(whiteBuffer, sizeof(whiteBuffer), "White: %02d:%02d", whiteMinutes, whiteSeconds);
            snprintf(blackBuffer, sizeof(blackBuffer), "Black: %02d:%02d", blackMinutes, blackSeconds);

            whiteTimerText.setString(whiteBuffer);
            blackTimerText.setString(blackBuffer);
            window.draw(whiteTimerText);
            window.draw(blackTimerText);
        }

        if (gameState != stateplaying && gameState != staterecords && fontLoaded)
        {
            sf::Text text;
            text.setFont(font);
            text.setCharacterSize(50);
            text.setFillColor(sf::Color::Red);
            text.setStyle(sf::Text::Bold);
            text.setPosition(windowwidth / 2 - 150, windowwidth / 2 - 25);

            if (gameState == statewhitewon)
                text.setString("White Wins!");
            else if (gameState == stateblackwon)
                text.setString("Black Wins!");
            else if (gameState == statestalemate)
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
        for (auto piece : capturedPieces)
        {
            if (piece)
            {
                delete piece;
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
