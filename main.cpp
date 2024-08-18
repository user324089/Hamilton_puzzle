#include "dimensions.hpp"

#include <array>
#include <cstdio>
#include <iostream>
#include <vector>
#include <windows.h>

constexpr int dummy_field = -2;

LRESULT CALLBACK window_callback(HWND, UINT, WPARAM, LPARAM);
TCHAR window_class_name[] = "Hamilton puzzle app";

int get_clicked_field(unsigned int smaller_dimention, unsigned int x, unsigned int y);

template <int n> struct numberLength {
    public:
        constexpr static int number = n >= 10 ? numberLength<n / 10>::number + 1 : 1;
};

template <> struct numberLength<0> {
    public:
        constexpr static int number = 1;
};

constexpr int field_length = numberLength<board_size - 1>::number;

int WINAPI WinMain(HINSTANCE this_instance, HINSTANCE, LPSTR, int nCmdShow) {
    HWND hwnd;
    MSG msg;
    WNDCLASSEX wincl;

    wincl.hInstance = this_instance;
    wincl.lpszClassName = window_class_name;
    // set the callback function
    wincl.lpfnWndProc = window_callback;
    // catch double clicks
    wincl.style = CS_DBLCLKS;
    wincl.cbSize = sizeof(WNDCLASSEX);

    // use default icon and mouse-pointer
    wincl.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    wincl.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wincl.lpszMenuName = nullptr;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;
    wincl.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    // register window class
    if (!RegisterClassEx(&wincl)) {
        return 0;
    }

    hwnd = CreateWindowEx(0, window_class_name, "Hamilton Puzzle", WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, // default position
                          CW_USEDEFAULT,
                          480 + GetSystemMetrics(SM_CXFRAME) * 2, // window dimensions
                          480 + GetSystemMetrics(SM_CYFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION),
                          HWND_DESKTOP, // set desktop as parent window
                          nullptr,      // no menu
                          this_instance, nullptr);

    ShowWindow(hwnd, nCmdShow);

    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // return the value of PostQuitMessage()
    return msg.wParam;
}

typedef std::array<char, side * side> level;

LRESULT CALLBACK window_callback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

    static int smaller_dimension;
    static int viewport_width, viewport_height;

    static std::vector<level> levels;
    static std::vector<level>::iterator current_level;


    static std::array<unsigned int, board_size + 1> path;
    static int path_length = 0;

    switch (message) {
        case WM_CREATE:
            {
                // get the data from levels resource
                HINSTANCE hInstance = ((LPCREATESTRUCT)lParam)->hInstance;

                HRSRC resource = FindResource(hInstance, "LEVELS", "TEXT");
                unsigned int num_levels = SizeofResource(hInstance, resource) / (board_size);
                static HGLOBAL levels_resource_handle = LoadResource(hInstance, resource);

                levels.resize(num_levels);
                char *level_data = (char *)LockResource(levels_resource_handle);

                // populate levels with the data
                for (unsigned int i = 0; i < num_levels; i++) {
                    for (unsigned int j = 0; j < board_size; j++) {
                        levels[i][j] = level_data[i * board_size + j];
                    }
                }

                FreeResource(levels_resource_handle);

                current_level = levels.begin();
            }
        case WM_PAINT:
            {
                InvalidateRect(hwnd, nullptr, FALSE);
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                HBITMAP hBitmap = CreateCompatibleBitmap(hdc, viewport_width, viewport_height);
                HDC hdcMem = CreateCompatibleDC(hdc);
                SelectObject(hdcMem, hBitmap);
                PatBlt(hdcMem, 0, 0, viewport_width, viewport_height, BLACKNESS);

                LOGFONT f;
                ZeroMemory(&f, sizeof(f));
                f.lfHeight = smaller_dimension / (side + 1);
                strcpy(f.lfFaceName, "Arial");
                HFONT hFont = CreateFontIndirect(&f);
                SelectObject(hdcMem, hFont);
                SetBkMode(hdcMem, TRANSPARENT);
                SetTextColor(hdcMem, RGB(0, 100, 255));

                SelectObject(hdcMem, CreateSolidBrush(RGB(0, 255, 255)));


                int block_side = smaller_dimension / (side + 1);
                int block_gap = block_side / (side + 1);

                // draw fields
                for (unsigned int y = 0; y < side; y++) {
                    for (unsigned int x = 0; x < side; x++) {
                        int block_top = block_side * y + block_gap * (y + 1);
                        int block_left = block_side * x + block_gap * (x + 1);
                        Rectangle(hdcMem, block_left, block_top, block_left + block_side,
                                  block_top + block_side);
                        char number[field_length + 1];
                        sprintf(number, "%i", (*current_level)[y * side + x]);
                        SIZE textSize;
                        GetTextExtentPoint32(hdcMem, number, strlen(number), &textSize);
                        TextOut(hdcMem,
                                smaller_dimension / (side + 1) * x
                                    + smaller_dimension / (side + 1) / 2
                                    + smaller_dimension / (side + 1) / (side + 1) * (x + 1)
                                    - textSize.cx / 2,
                                smaller_dimension / (side + 1) * y
                                    + smaller_dimension / (side + 1) / (side + 1) * (y + 1),
                                number, strlen(number));
                    }
                }

                // draw path
                if (path_length > 1) {
                    SelectObject(hdcMem, CreatePen(PS_SOLID, smaller_dimension / (side + 1) / 10,
                                                   RGB(255, 0, 0)));
                    int firstField = path[0];
                    MoveToEx(hdcMem,
                             block_side * (firstField % side) + block_side / 2
                                 + block_gap * ((firstField % side + 1)),
                             block_side * (firstField / side) + block_side / 2
                                 + block_gap * ((firstField / side + 1)),
                             nullptr);
                    for (int i = 0; i < path_length; i++) {
                        LineTo(hdcMem,
                               block_side * (path[i] % side) + block_side / 2
                                   + block_gap * ((path[i] % side + 1)),
                               block_side * (path[i] / side) + block_side / 2
                                   + block_gap * ((path[i] / side + 1)));
                    }
                    DeleteObject(SelectObject(hdcMem, GetStockObject(BLACK_PEN)));
                }


                DeleteObject(hFont);
                DeleteObject(SelectObject(hdcMem, GetStockObject(BLACK_BRUSH)));

                BitBlt(hdc, 0, 0, viewport_width, viewport_height, hdcMem, 0, 0, SRCCOPY);
                DeleteDC(hdcMem);
                DeleteObject(hBitmap);
                EndPaint(hwnd, &ps);
            }
            break;
        case WM_LBUTTONDOWN:
            {
                // start following a path
                int clicked_field =
                    get_clicked_field(smaller_dimension, LOWORD(lParam), HIWORD(lParam));
                if (clicked_field != dummy_field) {
                    path_length = 1;
                    path[0] = clicked_field;
                    SetCapture(hwnd);
                }
            }
            break;
        case WM_LBUTTONUP:
            // stop following a path
            if ((path_length != side * side + 1) || path[0] != path[path_length - 1]) {
                path_length = 0;
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            ReleaseCapture();
            break;
        case WM_MOUSEMOVE:
            // only take effect if the path is being drawn
            if (path_length != 0 && path_length != side * side + 1) {
                unsigned int currentField = path[path_length - 1];
                int mouse_field =
                    get_clicked_field(smaller_dimension, LOWORD(lParam), HIWORD(lParam));
                bool can_be_lengthened = true;

                if (mouse_field != dummy_field) {

                    unsigned int mouse_field_u = static_cast<unsigned int>(mouse_field);

                    // if a path would cross itself, shorten it so it doesn't

                    // prevent crossing the first tile only if it would not complete the cycle
                    int starting_cross_check_index = 0;
                    if (path_length == side * side) {
                        starting_cross_check_index = 1;
                    }

                    for (int i = starting_cross_check_index; i < path_length - 1; i++) {
                        if (path[i] == mouse_field_u) {
                            can_be_lengthened = false;
                            path_length = i + 1;
                            break;
                        }
                    }
                    // check if the chosen field is valid
                    if (can_be_lengthened
                        && ((mouse_field_u % side > 0 && mouse_field_u - 1 == currentField)
                            || (mouse_field_u % side < (side - 1)
                                && mouse_field_u + 1 == currentField)
                            || (mouse_field_u / side > 0 && mouse_field_u - side == currentField)
                            || (mouse_field_u / side < (side - 1)
                                && mouse_field_u + side == currentField))) {
                        path[path_length] = mouse_field_u;
                        path_length++;
                    }
                    InvalidateRect(hwnd, nullptr, FALSE);
                }
            }
            break;
        case WM_MOUSEWHEEL:
            {
                // only take effect if the path is complete
                if (path_length == side * side + 1) {
                    // change sliding direction depending on the scroll direction
                    if ((char)HIWORD(wParam) > 0) {
                        int temp = (*current_level)[path[0]];
                        for (int i = 0; i < path_length - 2; i++) {
                            (*current_level)[path[i]] =
                                (*current_level)[path[(i + 1) % (side * side)]];
                        }
                        (*current_level)[path[(path_length - 2) % (side * side)]] = temp;
                    } else {
                        int temp = (*current_level)[path[0]];
                        for (int i = path_length - 1; i > 1; i--) {
                            (*current_level)[path[i]] =
                                (*current_level)[path[(i + side * side - 1) % (side * side)]];
                        }
                        (*current_level)[path[1]] = temp;
                    }
                }
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            break;
        case WM_SIZE:
            // save the window dimensions for later events
            viewport_width = LOWORD(lParam);
            viewport_height = HIWORD(lParam);
            smaller_dimension = std::min(viewport_height, viewport_width);
            InvalidateRect(hwnd, nullptr, FALSE);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            // handle default events
            return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}

int get_clicked_field(unsigned int smaller_dimention, unsigned int x, unsigned int y) {

    unsigned int block_with_margin_side = smaller_dimention / (side);
    unsigned int field_x = x / block_with_margin_side, field_y = y / block_with_margin_side;

    // check if the field is a valid board field
    if (field_x < side && field_y < side) {
        return field_y * side + field_x;
    }

    // return a dummy field that does not coincide with any real one
    return dummy_field;
}
