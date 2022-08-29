// cyborgs.cpp

#include <iostream>
#include <string>
#include <random>
#include <utility>
#include <cstdlib>
#include <cctype>
#include <cassert>
using namespace std;


///////////////////////////////////////////////////////////////////////////
// Manifest constants
///////////////////////////////////////////////////////////////////////////

const int MAXROWS = 20;              // max number of rows in the arena
const int MAXCOLS = 20;              // max number of columns in the arena
const int MAXCYBORGS = 100;          // max number of cyborgs allowed
const int MAXCHANNELS = 3;           // max number of channels
const int INITIAL_CYBORG_HEALTH = 3; // initial cyborg health
const double WALL_DENSITY = 0.11;    // density of walls

const int NORTH = 0;
const int EAST = 1;
const int SOUTH = 2;
const int WEST = 3;
const int NUMDIRS = 4;
const int BADDIR = -1;

///////////////////////////////////////////////////////////////////////////
// Type definitions
///////////////////////////////////////////////////////////////////////////

class Arena;  // This is needed to let the compiler know that Arena is a
              // type name, since it's mentioned in the Cyborg declaration.

class Cyborg
{
public:
    // Constructor
    Cyborg(Arena* ap, int r, int c, int channel);

    // Accessors
    int  row() const;
    int  col() const;
    int  channel() const;
    bool isDead() const;

    // Mutators
    void forceMove(int dir);
    void move();

private:
    Arena* m_arena;
    int    m_row;
    int    m_col;
    int    m_channel;
    int    m_health;
};

class Player
{
public:
    // Constructor
    Player(Arena* ap, int r, int c);

    // Accessors
    int  row() const;
    int  col() const;
    bool isDead() const;

    // Mutators
    string stand();
    string move(int dir);
    void   setDead();

private:
    Arena* m_arena;
    int    m_row;
    int    m_col;
    bool   m_dead;
};

class Arena
{
public:
    // Constructor/destructor
    Arena(int nRows, int nCols);
    ~Arena();

    // Accessors
    int     rows() const;
    int     cols() const;
    Player* player() const;
    int     cyborgCount() const;
    bool    hasWallAt(int r, int c) const;
    int     numberOfCyborgsAt(int r, int c) const;
    void    display(string msg) const;

    // Mutators
    void   placeWallAt(int r, int c);
    bool   addCyborg(int r, int c, int channel);
    bool   addPlayer(int r, int c);
    string moveCyborgs(int channel, int dir);

private:
    bool    m_wallGrid[MAXROWS][MAXCOLS];
    int     m_rows;
    int     m_cols;
    Player* m_player;
    Cyborg* m_cyborgs[MAXCYBORGS];
    int     m_nCyborgs;

    // Helper functions
    void checkPos(int r, int c, string functionName) const;
    bool isPosInBounds(int r, int c) const;
};

class Game
{
public:
    // Constructor/destructor
    Game(int rows, int cols, int nCyborgs);
    ~Game();

    // Mutators
    void play();

private:
    Arena* m_arena;

    // Helper functions
    string takePlayerTurn();
    string takeCyborgsTurn();
};

///////////////////////////////////////////////////////////////////////////
//  Auxiliary function declarations
///////////////////////////////////////////////////////////////////////////

int decodeDirection(char ch);
int randInt(int lowest, int highest);
bool attemptMove(const Arena& a, int dir, int& r, int& c);
bool recommendMove(const Arena& a, int r, int c, int& bestDir);
void clearScreen();

///////////////////////////////////////////////////////////////////////////
//  Cyborg implementation
///////////////////////////////////////////////////////////////////////////

Cyborg::Cyborg(Arena* ap, int r, int c, int channel)
{
    if (ap == nullptr)
    {
        cout << "***** A cyborg must be created in some Arena!" << endl;
        exit(1);
    }
    if (r < 1 || r > ap->rows() || c < 1 || c > ap->cols())
    {
        cout << "***** Cyborg created with invalid coordinates (" << r << ","
            << c << ")!" << endl;
        exit(1);
    }
    if (channel < 1 || channel > MAXCHANNELS)
    {
        cout << "***** Cyborg created with invalid channel " << channel << endl;
        exit(1);
    }
    m_arena = ap;
    m_row = r;
    m_col = c;
    m_channel = channel;
    m_health = INITIAL_CYBORG_HEALTH;
}

int Cyborg::row() const
{
    return m_row;
}

int Cyborg::col() const
{
    return m_col;
}

int Cyborg::channel() const
{
    return m_channel; 
}

bool Cyborg::isDead() const
{
    if (m_health <= 0)
        return true;
    return false;
}

void Cyborg::forceMove(int dir)
{
    if (dir == 0)
    {
        if (m_row - 1 < 1 || m_arena->hasWallAt(m_row - 1, m_col))
            m_health--;
        else  
            m_row--;
    }
    else if (dir == 1)
    {
        if (m_col + 1 > m_arena->cols() || m_arena->hasWallAt(m_row, m_col + 1))
            m_health--;
        else 
            m_col++;
    }
    else if (dir == 2)
    {
        if (m_row + 1 > m_arena->rows() || m_arena->hasWallAt(m_row + 1, m_col))
            m_health--;
        else
            m_row++;
    }
    else if (dir == 3)
    {
        if (m_col - 1 < 1 || m_arena->hasWallAt(m_row, m_col - 1))
            m_health--;
        else
            m_col--;
    }        
}

void Cyborg::move()
{
    if (!isDead())
        attemptMove(*m_arena, randInt(0, NUMDIRS - 1), m_row, m_col);
}

///////////////////////////////////////////////////////////////////////////
//  Player implementation
///////////////////////////////////////////////////////////////////////////

Player::Player(Arena* ap, int r, int c)
{
    if (ap == nullptr)
    {
        cout << "***** The player must be created in some Arena!" << endl;
        exit(1);
    }
    if (r < 1 || r > ap->rows() || c < 1 || c > ap->cols())
    {
        cout << "**** Player created with invalid coordinates (" << r
            << "," << c << ")!" << endl;
        exit(1);
    }
    m_arena = ap;
    m_row = r;
    m_col = c;
    m_dead = false;
}

int Player::row() const
{
    return m_row;  
}

int Player::col() const
{
    return m_col; 
}

string Player::stand()
{
    return "Player stands.";
}

string Player::move(int dir)
{
    if (dir == 0) 
    {
        if (m_row - 1 < 1 || m_arena->hasWallAt(m_row - 1, m_col)) 
        {
            return "Player couldn't move; player stands.";
        }
        m_row--;
        if (m_arena->numberOfCyborgsAt(m_row, m_col) == 0)
            return "Player moved north.";
        else
        {
            setDead();
            return "Player walked into a cybord and died.";
        }
    }
    else if (dir == 1)
    {
        if (m_col + 1 > m_arena->cols() || m_arena->hasWallAt(m_row, m_col + 1))
        {
            return "Player couldn't move; player stands.";
        }
        m_col++;
        if (m_arena->numberOfCyborgsAt(m_row, m_col) == 0)
            return "Player moved east.";
        else
        {
            setDead();
            return "Player walked into a cybord and died.";
        }
    }
    else if (dir == 2)
    {
        if (m_row + 1 > m_arena->rows() || m_arena->hasWallAt(m_row + 1, m_col))
        {
            return "Player couldn't move; player stands.";
        }
        m_row++;
        if (m_arena->numberOfCyborgsAt(m_row, m_col) == 0)
            return "Player moved south.";
        else
        {
            setDead();
            return "Player walked into a cybord and died.";
        }
    }
    else if (dir == 3)
    {
        if (m_col - 1 < 1 || m_arena->hasWallAt(m_row, m_col - 1))
        {
            return "Player couldn't move; player stands.";
        }
        m_col--;
        if (m_arena->numberOfCyborgsAt(m_row, m_col) == 0)
            return "Player moved west.";
        else
        {
            setDead();
            return "Player walked into a cybord and died.";
        }
    }
    return "0";
}

bool Player::isDead() const
{
    if (m_dead)
        return true; 
    return false;
}

void Player::setDead()
{
    m_dead = true;
}

///////////////////////////////////////////////////////////////////////////
//  Arena implementation
///////////////////////////////////////////////////////////////////////////

Arena::Arena(int nRows, int nCols)
{
    if (nRows <= 0 || nCols <= 0 || nRows > MAXROWS || nCols > MAXCOLS)
    {
        cout << "***** Arena created with invalid size " << nRows << " by "
            << nCols << "!" << endl;
        exit(1);
    }
    m_rows = nRows;
    m_cols = nCols;
    m_player = nullptr;
    m_nCyborgs = 0;
    for (int r = 1; r <= m_rows; r++)
        for (int c = 1; c <= m_cols; c++)
            m_wallGrid[r - 1][c - 1] = false;
}

Arena::~Arena()
{
    delete m_player;
    for (size_t i = 0; i < m_nCyborgs; i++)
        delete m_cyborgs[i];
}

int Arena::rows() const
{
    return m_rows;  
}

int Arena::cols() const
{
    return m_cols; 
}

Player* Arena::player() const
{
    return m_player;
}

int Arena::cyborgCount() const
{
    return m_nCyborgs; 
}

bool Arena::hasWallAt(int r, int c) const
{
    checkPos(r, c, "Arena::hasWallAt");
    return m_wallGrid[r - 1][c - 1];
}

int Arena::numberOfCyborgsAt(int r, int c) const
{
    int num = 0;
    for (size_t i = 0; i < m_nCyborgs; i++)
    {
        if (m_cyborgs[i]->row() == r && m_cyborgs[i]->col() == c)
            num++;
    }
    return num;
}

void Arena::display(string msg) const
{
    char displayGrid[MAXROWS][MAXCOLS];
    int r, c;

    // Fill displayGrid with dots (empty) and stars (wall)
    for (r = 1; r <= rows(); r++)
        for (c = 1; c <= cols(); c++)
            displayGrid[r - 1][c - 1] = (hasWallAt(r, c) ? '*' : '.');

    for (size_t i = 0; i < m_nCyborgs; i++)
    {
        if (m_cyborgs[i] != nullptr) 
        {
            if (m_cyborgs[i]->channel() == 1)
            {
                displayGrid[m_cyborgs[i]->row() - 1][m_cyborgs[i]->col() - 1] = '1';
            }
            else if (m_cyborgs[i]->channel() == 2)
            {
                displayGrid[m_cyborgs[i]->row() - 1][m_cyborgs[i]->col() - 1] = '2';
            }
            else if (m_cyborgs[i]->channel() == 3)
            {
                displayGrid[m_cyborgs[i]->row() - 1][m_cyborgs[i]->col() - 1] = '3';
            }
        }
            
    }

    // Indicate player's position
    if (m_player != nullptr)
        displayGrid[m_player->row() - 1][m_player->col() - 1] = (m_player->isDead() ? 'X' : '@');

    // Draw the grid
    clearScreen();
    for (r = 1; r <= rows(); r++)
    {
        for (c = 1; c <= cols(); c++)
            cout << displayGrid[r - 1][c - 1];
        cout << endl;
    }
    cout << endl;

    // Write message, cyborg, and player info
    if (msg != "")
        cout << msg << endl;
    cout << "There are " << cyborgCount() << " cyborgs remaining." << endl;
    if (m_player == nullptr)
        cout << "There is no player!" << endl;
    else if (m_player->isDead())
        cout << "The player is dead." << endl;
}

void Arena::placeWallAt(int r, int c)
{
    checkPos(r, c, "Arena::placeWallAt");
    m_wallGrid[r - 1][c - 1] = true;
}

bool Arena::addCyborg(int r, int c, int channel)
{
    if (!isPosInBounds(r, c) || hasWallAt(r, c))
        return false;
    if (m_player != nullptr && m_player->row() == r && m_player->col() == c)
        return false;
    if (channel < 1 || channel > MAXCHANNELS)
        return false;
    if (m_nCyborgs == MAXCYBORGS)
        return false;
    m_cyborgs[m_nCyborgs] = new Cyborg(this, r, c, channel);
    m_nCyborgs++;
    return true;
}

bool Arena::addPlayer(int r, int c)
{
    if (m_player != nullptr || !isPosInBounds(r, c) || hasWallAt(r, c))
        return false;
    if (numberOfCyborgsAt(r, c) > 0)
        return false;
    m_player = new Player(this, r, c);
    return true;
}

string Arena::moveCyborgs(int channel, int dir)
{
    // Cyborgs on the channel will respond with probability 1/2
    bool willRespond = (randInt(0, 1) == 0);

    // Move all cyborgs
    int nCyborgsOriginally = m_nCyborgs;

    if (willRespond == true) 
    {
        for (size_t i = 0; i < m_nCyborgs; i++)
        {
            if (m_cyborgs[i]->channel() == channel)
                m_cyborgs[i]->forceMove(dir);
            else
                m_cyborgs[i]->move();
        }
    }
    else if (willRespond == false)
    {
        for (size_t i = 0; i < m_nCyborgs; i++)
            m_cyborgs[i]->move();
    }
    for (size_t i = 0; i < m_nCyborgs; i++)
    {
        if (m_cyborgs[i]->isDead())
        {
            size_t k = i;
            size_t j = i + 1;
            Cyborg* temp[MAXCYBORGS];
            while (j != m_nCyborgs)
            {
                temp[i] = m_cyborgs[k];
                m_cyborgs[k] = m_cyborgs[j];
                m_cyborgs[j] = temp[i];
                j++;
                k++;
            }
            delete m_cyborgs[m_nCyborgs - 1];
            m_nCyborgs--;           
        }
        if (m_cyborgs[i]->row() == m_player->row() && m_cyborgs[i]->col() == m_player->col())
        {
            m_player->setDead();
        }
    }

    if (m_nCyborgs < nCyborgsOriginally)
        return "Some cyborgs have been destroyed.";
    else
        return "No cyborgs were destroyed.";
}

bool Arena::isPosInBounds(int r, int c) const
{
    return (r >= 1 && r <= m_rows && c >= 1 && c <= m_cols);
}

void Arena::checkPos(int r, int c, string functionName) const
{
    if (!isPosInBounds(r, c))
    {
        cout << "***** " << "Invalid arena position (" << r << ","
            << c << ") in call to " << functionName << endl;
        exit(1);
    }
}

///////////////////////////////////////////////////////////////////////////
//  Game implementation
///////////////////////////////////////////////////////////////////////////

Game::Game(int rows, int cols, int nCyborgs)
{
    if (nCyborgs < 0 || nCyborgs > MAXCYBORGS)
    {
        cout << "***** Game created with invalid number of cyborgs:  "
            << nCyborgs << endl;
        exit(1);
    }
    int nEmpty = rows * cols - nCyborgs - 1;  // 1 for Player
    if (nEmpty < 0)
    {
        cout << "***** Game created with a " << rows << " by "
            << cols << " arena, which is too small too hold a player and "
            << nCyborgs << " cyborgs!" << endl;
        exit(1);
    }

    // Create arena
    m_arena = new Arena(rows, cols);

    // Add some walls in WALL_DENSITY of the empty spots
    assert(WALL_DENSITY >= 0 && WALL_DENSITY <= 1);
    int nWalls = static_cast<int>(WALL_DENSITY * nEmpty);
    while (nWalls > 0)
    {
        int r = randInt(1, rows);
        int c = randInt(1, cols);
        if (m_arena->hasWallAt(r, c))
            continue;
        m_arena->placeWallAt(r, c);
        nWalls--;
    }

    // Add player
    int rPlayer;
    int cPlayer;
    do
    {
        rPlayer = randInt(1, rows);
        cPlayer = randInt(1, cols);
    } while (m_arena->hasWallAt(rPlayer, cPlayer));
    m_arena->addPlayer(rPlayer, cPlayer);

    // Populate with cyborgs
    while (nCyborgs > 0)
    {
        int r = randInt(1, rows);
        int c = randInt(1, cols);
        if (m_arena->hasWallAt(r, c) || (r == rPlayer && c == cPlayer))
            continue;
        m_arena->addCyborg(r, c, randInt(1, MAXCHANNELS));
        nCyborgs--;
    }
}

Game::~Game()
{
    delete m_arena;
}

string Game::takePlayerTurn()
{
    for (;;)
    {
        cout << "Your move (n/e/s/w/x or nothing): ";
        string playerMove;
        getline(cin, playerMove);

        Player* player = m_arena->player();
        int dir;

        if (playerMove.size() == 0)
        {
            if (recommendMove(*m_arena, player->row(), player->col(), dir))
                return player->move(dir);
            else
                return player->stand();
        }
        else if (playerMove.size() == 1)
        {
            if (tolower(playerMove[0]) == 'x')
                return player->stand();
            else
            {
                dir = decodeDirection(tolower(playerMove[0]));
                if (dir != BADDIR)
                    return player->move(dir);
            }
        }
        cout << "Player move must be nothing, or 1 character n/e/s/w/x." << endl;
    }
}

string Game::takeCyborgsTurn()
{
    for (;;)
    {
        cout << "Broadcast (e.g., 2n): ";
        string broadcast;
        getline(cin, broadcast);
        if (broadcast.size() != 2)
        {
            cout << "You must specify a channel followed by a direction." << endl;
            continue;
        }
        else if (broadcast[0] < '1' || broadcast[0] > '0' + MAXCHANNELS)
            cout << "Channel must be a digit in the range 1 through "
            << MAXCHANNELS << "." << endl;
        else
        {
            int dir = decodeDirection(tolower(broadcast[1]));
            if (dir == BADDIR)
                cout << "Direction must be n, e, s, or w." << endl;
            else
                return m_arena->moveCyborgs(broadcast[0] - '0', dir);
        }
    }
}

void Game::play()
{
    m_arena->display("");
    Player* player = m_arena->player();
    if (player == nullptr)
        return;
    while (!player->isDead() && m_arena->cyborgCount() > 0)
    {
        string msg = takePlayerTurn();
        m_arena->display(msg);
        if (player->isDead())
            break;
        msg = takeCyborgsTurn();
        m_arena->display(msg);
    }
    if (player->isDead())
        cout << "You lose." << endl;
    else
        cout << "You win." << endl;
}

///////////////////////////////////////////////////////////////////////////
//  Auxiliary function implementations
///////////////////////////////////////////////////////////////////////////

int decodeDirection(char dir)
{
    switch (dir)
    {
    case 'n':  return NORTH;
    case 'e':  return EAST;
    case 's':  return SOUTH;
    case 'w':  return WEST;
    }
    return BADDIR;  // bad argument passed in!
}

// Return a random int from min to max, inclusive
int randInt(int min, int max)
{
    if (max < min)
        swap(max, min);
    static random_device rd;
    static default_random_engine generator(rd());
    uniform_int_distribution<> distro(min, max);
    return distro(generator);
}

bool attemptMove(const Arena& a, int dir, int& r, int& c)
{
    if (dir == 0)
    {
        if (r - 1 < 1 || a.hasWallAt(r - 1, c))
            return false;
        else
            r--;
    }
    else if (dir == 1)
    {
        if (c + 1 > a.cols() || a.hasWallAt(r, c + 1))
            return false;
        else
            c++;
    }
    else if (dir == 2)
    {
        if (r + 1 > a.rows() || a.hasWallAt(r + 1, c))
            return false;
        else
            r++;
    }
    else if (dir == 3)
    {
        if (c - 1 < 1 || a.hasWallAt(r, c - 1))
            return false;
        else
            c--;
    }
    return true;
}

// Recommend a move for a player at (r,c): 
bool recommendMove(const Arena& a, int r, int c, int& bestDir)
{
    int tot = 0;
    int closestCyb[NUMDIRS + 1];
    size_t j = 0;
    for (size_t i = r - 1; i != 0 ; i--)
    {
        if (a.numberOfCyborgsAt(i, c) != 0 || a.hasWallAt(i, c)) 
        {
            if (j == 0)
            {
                closestCyb[j] = r - i;
                j++;
            }
        }           
        tot++;
    }
    if (j == 0) {
        closestCyb[j] = tot;
        j++;
    }
    tot = 0;
    for (size_t i = c + 1; i <= a.cols(); i++)
    {
        if (a.numberOfCyborgsAt(r, i) != 0 || a.hasWallAt(r, i))
        {
            if (j == 1)
            {
                closestCyb[j] = i - c;
                j++;
            }
        }
        tot++;
    }
    if (j == 1) {
        closestCyb[j] = tot;
        j++;
    }
    tot = 0;
    for (size_t i = r + 1; i <= a.rows(); i++)
    {
        if (a.numberOfCyborgsAt(i, c) != 0 || a.hasWallAt(i, c))
        {
            if (j == 2)
            {
                closestCyb[j] = i - r;
                j++;
            }

        }
        tot++;
    }
    if (j == 2) {
        closestCyb[j] = tot;
        j++;
    }
    tot = 0;
    for (size_t i = c - 1; i != 0; i--)
    {
        if (a.numberOfCyborgsAt(r, i) != 0 || a.hasWallAt(r, i))
        {
            if (j == 3)
            {
                closestCyb[j] = c - i;
                j++;
            }
        }
        tot++;
    }
    if (j == 3) {
        closestCyb[j] = tot;
        j++;
    }
    tot = 0;


    if (closestCyb[0] == closestCyb[1] && closestCyb[0] == closestCyb[2] && closestCyb[0] == closestCyb[3])
        return false;

    int max = 0;
    for (size_t i = 0; i < NUMDIRS; i++)
    {
        size_t j = i;
        while (j != NUMDIRS)
        {
            if (closestCyb[j] > closestCyb[max])
                max = j;
            j++;
        }
    }
    bestDir = max;
    return true;
}

///////////////////////////////////////////////////////////////////////////
// main()
///////////////////////////////////////////////////////////////////////////

int main()
{
  // Game g(width, height, # of cyborgs) 
    Game g(3, 5, 4);

    g.play();
}

///////////////////////////////////////////////////////////////////////////
//  clearScreen implementation
///////////////////////////////////////////////////////////////////////////

// DO NOT MODIFY OR REMOVE ANYTHING BETWEEN HERE AND THE END OF THE FILE!!!
// THE CODE IS SUITABLE FOR VISUAL C++, XCODE, AND g++/g31 UNDER LINUX.

// Note to Xcode users:  clearScreen() will just write a newline instead
// of clearing the window if you launch your program from within Xcode.
// That's acceptable.  (The Xcode output window doesn't have the capability
// of being cleared.)

#ifdef _WIN32

#pragma warning(disable : 4005)
#include <windows.h>

void clearScreen()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    DWORD dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
    COORD upperLeft = { 0, 0 };
    DWORD dwCharsWritten;
    FillConsoleOutputCharacter(hConsole, TCHAR(' '), dwConSize, upperLeft,
        &dwCharsWritten);
    SetConsoleCursorPosition(hConsole, upperLeft);
}

#else  // not _WIN32

#include <iostream>
#include <cstring>
#include <cstdlib>

void clearScreen()  // will just write a newline in an Xcode output window
{
    static const char* term = getenv("TERM");
    if (term == nullptr || strcmp(term, "dumb") == 0)
        cout << endl;
    else
    {
        static const char* ESC_SEQ = "\x1B[";  // ANSI Terminal esc seq:  ESC [
        cout << ESC_SEQ << "2J" << ESC_SEQ << "H" << flush;
    }
}

#endif
