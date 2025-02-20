#include <windows.h>
#include <vector>
#include <deque>
#include <random>
#include <string>

using namespace std;

// Design-Konstanten
const COLORREF COLOR_BG = RGB(18, 18, 18);           // Dunkler Hintergrund
const COLORREF COLOR_GRID = RGB(32, 32, 32);         // Gitternetz
const COLORREF COLOR_SNAKE = RGB(0, 255, 128);       // Neon-Grün für Schlange
const COLORREF COLOR_SNAKE_HEAD = RGB(0, 255, 200);  // Helleres Grün für Kopf
const COLORREF COLOR_FOOD = RGB(255, 64, 64);        // Leuchtendes Rot
const COLORREF COLOR_TEXT = RGB(240, 240, 240);      // Helles Weiß für Text
const COLORREF COLOR_BUTTON = RGB(45, 45, 45);       // Dunkelgrau für Buttons
const COLORREF COLOR_BUTTON_HOVER = RGB(60, 60, 60); // Helleres Grau für Hover

// Fenstereinstellungen
const char* WINDOW_CLASS = "ModernSnakeGameClass";
const char* WINDOW_TITLE = "Modern Snake";
const int CELL_SIZE = 25;  // Größere Zellen
const int GRID_WIDTH = 30;
const int GRID_HEIGHT = 20;
const int WINDOW_WIDTH = CELL_SIZE * GRID_WIDTH;
const int WINDOW_HEIGHT = CELL_SIZE * GRID_HEIGHT;

// Spielzustände
enum GameState {
    MENU,
    PLAYING,
    GAME_OVER
};

// Menü-Optionen
enum MenuOption {
    PLAY,
    EXIT,
    MENU_OPTIONS_COUNT
};

// Richtungen
enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };

// Button-Definitionen
RECT menuButtons[2];
DWORD gameOverStartTime = 0;

// Globale Variablen
GameState gameState = MENU;
MenuOption selectedOption = PLAY;
bool gameOver = false;
Direction direction = STOP;
deque<pair<int, int>> snake;
pair<int, int> food;
int score = 0;
UINT_PTR timerId;

// Hilfsfunktion für abgerundete Rechtecke
void DrawRoundRect(HDC hdc, RECT rect, int radius, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    HPEN pen = CreatePen(PS_SOLID, 1, color);
    SelectObject(hdc, brush);
    SelectObject(hdc, pen);

    RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, radius, radius);

    DeleteObject(brush);
    DeleteObject(pen);
}

// Hilfsfunktion für Schatten-Effekt
void DrawShadowRect(HDC hdc, RECT rect, int radius) {
    for(int i = 0; i < 5; i++) {
        RECT shadowRect = rect;
        InflateRect(&shadowRect, i, i);
        COLORREF shadowColor = RGB(0, 0, 0);
        BYTE alpha = 50 - i * 10;
        DrawRoundRect(hdc, shadowRect, radius, shadowColor);
    }
}

// Futter platzieren
void PlaceFood() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> disX(0, GRID_WIDTH - 1);
    uniform_int_distribution<> disY(0, GRID_HEIGHT - 1);
    food = {disX(gen), disY(gen)};
}

// Spiel zurücksetzen
void ResetGame() {
    gameOver = false;
    direction = STOP;
    score = 0;
    snake.clear();
    snake.push_front({GRID_WIDTH/2, GRID_HEIGHT/2});
    PlaceFood();
    gameOverStartTime = 0;
}

// Spiellogik
void GameLogic() {
    if (gameOver || direction == STOP) return;

    auto head = snake.front();
    switch(direction) {
        case UP: head.second--; break;
        case DOWN: head.second++; break;
        case LEFT: head.first--; break;
        case RIGHT: head.first++; break;
    }

    if (head.first < 0 || head.first >= GRID_WIDTH ||
        head.second < 0 || head.second >= GRID_HEIGHT) {
        gameState = GAME_OVER;
        gameOverStartTime = GetTickCount();
        return;
    }

    for (const auto& segment : snake) {
        if (head.first == segment.first && head.second == segment.second) {
            gameState = GAME_OVER;
            gameOverStartTime = GetTickCount();
            return;
        }
    }

    snake.push_front(head);

    if (head.first == food.first && head.second == food.second) {
        score += 10;
        PlaceFood();
    } else {
        snake.pop_back();
    }
}

// Menü zeichnen
void DrawMenu(HDC hdc) {
    // Hintergrund mit Gitternetz
    RECT rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    HBRUSH bgBrush = CreateSolidBrush(COLOR_BG);
    FillRect(hdc, &rect, bgBrush);
    DeleteObject(bgBrush);

    // Gitternetz zeichnen
    HPEN gridPen = CreatePen(PS_SOLID, 1, COLOR_GRID);
    SelectObject(hdc, gridPen);
    for(int x = 0; x < WINDOW_WIDTH; x += CELL_SIZE) {
        MoveToEx(hdc, x, 0, NULL);
        LineTo(hdc, x, WINDOW_HEIGHT);
    }
    for(int y = 0; y < WINDOW_HEIGHT; y += CELL_SIZE) {
        MoveToEx(hdc, 0, y, NULL);
        LineTo(hdc, WINDOW_WIDTH, y);
    }
    DeleteObject(gridPen);

    // Titel mit Schatten
    SetBkMode(hdc, TRANSPARENT);
    HFONT titleFont = CreateFontA(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                 ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    SelectObject(hdc, titleFont);

    // Schatten für Titel
    SetTextColor(hdc, RGB(0, 0, 0));
    RECT shadowRect = {2, WINDOW_HEIGHT/4 + 2, WINDOW_WIDTH + 2, WINDOW_HEIGHT/2 + 2};
    DrawTextA(hdc, "MODERN SNAKE", -1, &shadowRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Titel selbst
    SetTextColor(hdc, COLOR_TEXT);
    RECT titleRect = {0, WINDOW_HEIGHT/4, WINDOW_WIDTH, WINDOW_HEIGHT/2};
    DrawTextA(hdc, "MODERN SNAKE", -1, &titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DeleteObject(titleFont);

    // Moderne Buttons
    const char* options[] = {"SPIELEN", "BEENDEN"};
    HFONT buttonFont = CreateFontA(24, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                  ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                  ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    SelectObject(hdc, buttonFont);

    for (int i = 0; i < MENU_OPTIONS_COUNT; i++) {
        menuButtons[i] = {WINDOW_WIDTH/2 - 120, WINDOW_HEIGHT/2 + i*70,
                         WINDOW_WIDTH/2 + 120, WINDOW_HEIGHT/2 + 50 + i*70};

        // Button Schatten
        DrawShadowRect(hdc, menuButtons[i], 10);

        // Button Hintergrund
        DrawRoundRect(hdc, menuButtons[i], 10,
                     i == selectedOption ? COLOR_BUTTON_HOVER : COLOR_BUTTON);

        // Button Text
        SetTextColor(hdc, COLOR_TEXT);
        DrawTextA(hdc, options[i], -1, &menuButtons[i], DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    DeleteObject(buttonFont);
}

// Spiel zeichnen
void DrawGame(HWND hwnd, HDC hdc) {
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, WINDOW_WIDTH, WINDOW_HEIGHT);
    SelectObject(memDC, memBitmap);

    if (gameState == MENU) {
        DrawMenu(memDC);
    }
    else {
        // Hintergrund mit Gitternetz
        RECT rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        HBRUSH bgBrush = CreateSolidBrush(COLOR_BG);
        FillRect(memDC, &rect, bgBrush);
        DeleteObject(bgBrush);

        // Gitternetz
        HPEN gridPen = CreatePen(PS_SOLID, 1, COLOR_GRID);
        SelectObject(memDC, gridPen);
        for(int x = 0; x < WINDOW_WIDTH; x += CELL_SIZE) {
            MoveToEx(memDC, x, 0, NULL);
            LineTo(memDC, x, WINDOW_HEIGHT);
        }
        for(int y = 0; y < WINDOW_HEIGHT; y += CELL_SIZE) {
            MoveToEx(memDC, 0, y, NULL);
            LineTo(memDC, WINDOW_WIDTH, y);
        }
        DeleteObject(gridPen);

        // Schlange mit Farbverlauf
        for (size_t i = 0; i < snake.size(); i++) {
            RECT snakeRect = {
                snake[i].first * CELL_SIZE + 2,
                snake[i].second * CELL_SIZE + 2,
                (snake[i].first + 1) * CELL_SIZE - 2,
                (snake[i].second + 1) * CELL_SIZE - 2
            };

            // Kopf anders färben
            COLORREF snakeColor = (i == 0) ? COLOR_SNAKE_HEAD : COLOR_SNAKE;
            DrawRoundRect(memDC, snakeRect, 5, snakeColor);
        }

        // Futter mit Gloweffekt
        RECT foodRect = {
            food.first * CELL_SIZE + 2,
            food.second * CELL_SIZE + 2,
            (food.first + 1) * CELL_SIZE - 2,
            (food.second + 1) * CELL_SIZE - 2
        };
        for(int i = 3; i >= 0; i--) {
            RECT glowRect = foodRect;
            InflateRect(&glowRect, i, i);
            DrawRoundRect(memDC, glowRect, 5,
                         RGB(min(255, GetRValue(COLOR_FOOD) + i*20),
                             min(255, GetGValue(COLOR_FOOD) + i*20),
                             min(255, GetBValue(COLOR_FOOD) + i*20)));
        }

        // Score mit modernem Design
        HFONT scoreFont = CreateFontA(24, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                    ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                    ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
        SelectObject(memDC, scoreFont);
        SetTextColor(memDC, COLOR_TEXT);
        SetBkMode(memDC, TRANSPARENT);

        string scoreText = "SCORE: " + to_string(score);
        RECT scoreRect = {20, 20, 200, 50};
        DrawTextA(memDC, scoreText.c_str(), -1, &scoreRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        DeleteObject(scoreFont);

        // Game Over Screen
        if (gameState == GAME_OVER) {
            // Halbtransparenter Overlay
            HBRUSH overlayBrush = CreateSolidBrush(RGB(0, 0, 0));
            RECT overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
            FillRect(memDC, &overlay, overlayBrush);
            DeleteObject(overlayBrush);

            HFONT gameOverFont = CreateFontA(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                           ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                           ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            SelectObject(memDC, gameOverFont);

            SetTextColor(memDC, COLOR_TEXT);
            RECT textRect = {0, WINDOW_HEIGHT/2 - 60, WINDOW_WIDTH, WINDOW_HEIGHT/2};
            DrawTextA(memDC, "GAME OVER", -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Final Score
            string finalScore = "Final Score: " + to_string(score);
            RECT scoreRect = {0, WINDOW_HEIGHT/2, WINDOW_WIDTH, WINDOW_HEIGHT/2 + 40};
            DrawTextA(memDC, finalScore.c_str(), -1, &scoreRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            DeleteObject(gameOverFont);

            if (GetTickCount() - gameOverStartTime > 2000) {
                gameState = MENU;
                selectedOption = PLAY;
                InvalidateRect(hwnd, NULL, TRUE);
            }
        }
    }

    BitBlt(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, memDC, 0, 0, SRCCOPY);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}

// Fensterprozedur
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            timerId = SetTimer(hwnd, 1, 100, nullptr);
            return 0;
        }

        case WM_DESTROY: {
            KillTimer(hwnd, timerId);
            PostQuitMessage(0);
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            DrawGame(hwnd, hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_TIMER: {
            if (gameState == PLAYING) {
                GameLogic();
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;
        }

        case WM_LBUTTONDOWN: {
            if (gameState == MENU) {
                POINT pt = {LOWORD(lParam), HIWORD(lParam)};
                for (int i = 0; i < MENU_OPTIONS_COUNT; i++) {
                    if (PtInRect(&menuButtons[i], pt)) {
                        if (i == PLAY) {
                            gameState = PLAYING;
                            ResetGame();
                        } else if (i == EXIT) {
                            DestroyWindow(hwnd);
                        }
                        InvalidateRect(hwnd, NULL, TRUE);
                        break;
                    }
                }
            }
            return 0;
        }

        case WM_MOUSEMOVE: {
            if (gameState == MENU) {
                POINT pt = {LOWORD(lParam), HIWORD(lParam)};
                for (int i = 0; i < MENU_OPTIONS_COUNT; i++) {
                    if (PtInRect(&menuButtons[i], pt)) {
                        if (selectedOption != i) {
                            selectedOption = (MenuOption)i;
                            InvalidateRect(hwnd, NULL, TRUE);
                        }
                        break;
                    }
                }
            }
            return 0;
        }

        case WM_KEYDOWN: {
            switch (gameState) {
                case MENU:
                    switch (wParam) {
                        case 'W': case 'w':
                            selectedOption = (MenuOption)((selectedOption - 1 + MENU_OPTIONS_COUNT) % MENU_OPTIONS_COUNT);
                            break;
                        case 'S': case 's':
                            selectedOption = (MenuOption)((selectedOption + 1) % MENU_OPTIONS_COUNT);
                            break;
                        case VK_RETURN: case VK_SPACE:
                            if (selectedOption == PLAY) {
                                gameState = PLAYING;
                                ResetGame();
                            } else if (selectedOption == EXIT) {
                                DestroyWindow(hwnd);
                            }
                            break;
                    }
                    break;

                case PLAYING:
                    switch (wParam) {
                        case 'W': case 'w': if (direction != DOWN)  direction = UP;    break;
                        case 'S': case 's': if (direction != UP)    direction = DOWN;  break;
                        case 'A': case 'a': if (direction != RIGHT) direction = LEFT;  break;
                        case 'D': case 'd': if (direction != LEFT)  direction = RIGHT; break;
                    }
                    break;

                case GAME_OVER:
                    if (wParam == VK_SPACE) {
                        gameState = MENU;
                        selectedOption = PLAY;
                    }
                    break;
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = WINDOW_CLASS;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassA(&wc);

    RECT windowRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    AdjustWindowRect(&windowRect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);

    HWND hwnd = CreateWindowExA(
        0,
        WINDOW_CLASS,
        WINDOW_TITLE,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (hwnd == nullptr) return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
