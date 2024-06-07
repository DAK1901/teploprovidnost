// L2.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "L2.h"
#include "math.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <process.h>

#define MAX_LOADSTRING 100
#define WM_CALC_FINISHED (WM_USER + 0)

CRITICAL_SECTION cs;
HANDLE hEvent;
HWND hwnd;

int xView, yView;
int a = 10, b = 10;
double hx = 0.8, hy = 0.8;
double ht = 0.0001;

double t;
int K = 800;
int T = 100000;

double A0[21][21];
wchar_t bufC1[5];
wchar_t bufC2[5];
int C1 = 1, C2 = 1;
double U[21][21];
double A[21][21];

double u0y(double y, double t) { return C2 * t * sin(3.14 * y / b); }
double ubx() { return 0; }
double uay(double y, double t) { return C2 * t * sin(3.14 * y / b); }
double u0x() { return 0; }
double f(double x, double y, double t) { return C1 * t * pow(2.71, (-(pow(x - a / 2, 2) + pow(y - b / 2, 2)))); }
double v() { return 0; }

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Input(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL                Line(HDC, int, int, int, int);
void                NextLawer(int t);
void                Thread(PVOID lpParam);
int*                Preobr(double x, double y, double z);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Разместите код здесь.

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_L2, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_L2));

    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_L2));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (CreateSolidBrush(RGB(80, 80, 80)));
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_L2);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc; //создаём контекст устройства
    PAINTSTRUCT ps; //создаём экземпляр структуры графического вывода
    HPEN hPen; //создаём перо

    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Разобрать выбор в меню:
            switch (wmId)
            {
            case ID_INPUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_INPUT), hWnd, Input);
                break;
            case ID_CREATE:
                InitializeCriticalSection(&cs);
                hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
                _beginthread(Thread, 0, NULL);
                hwnd = hWnd;
                break;
            case ID_START:
                SetEvent(hEvent);
                break;
            case ID_DESTROY:
                DeleteCriticalSection(&cs);
                break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_SIZE:
        xView = LOWORD(lParam);
        yView = HIWORD(lParam);
        break;
    case WM_PAINT:
        {
        hdc = BeginPaint(hWnd, &ps);
        SetMapMode(hdc, MM_ISOTROPIC); //логические единицы отображаем, как физические
        SetWindowExtEx(hdc, 1000, 1000, NULL); //Длина осей
        SetViewportExtEx(hdc, xView, -yView, NULL); //Определяем облась вывода
        SetViewportOrgEx(hdc, xView / 2, yView / 2, NULL); //Начало координат

        hPen = CreatePen(1, 4, RGB(255, 255, 255));
        SelectObject(hdc, hPen);

        //Рисуем оси координат
        Line(hdc, 0, 200, 0, 500);//ось T
        Line(hdc, 0, 200, 800 * sqrt(3) / 2, -800 / sqrt(3) + 200);//ось y
        Line(hdc, 0, 200, -800 * sqrt(3) / 2, -800 / sqrt(3) + 200);//ось x
        MoveToEx(hdc, 0, 0, NULL); //перемещаемся в начало координат

        ValidateRect(hWnd, NULL); //Обновляем экран

        EndPaint(hWnd, &ps);
        }
        break;

    case WM_CALC_FINISHED:
        double A[21][21];
        hdc = GetDC(hWnd);
        SetMapMode(hdc, MM_ISOTROPIC); //логические единицы отображаем, как физические
        SetWindowExtEx(hdc, 1000, 1000, NULL); //Длина осей
        SetViewportExtEx(hdc, xView, -yView, NULL); //Определяем облась вывода
        SetViewportOrgEx(hdc, xView / 2, yView / 2, NULL); //Начало координат

        RECT rect;

        rect.left = -1000;
        rect.right = 1000;
        rect.top = 1000;
        rect.bottom = -1000;

        FillRect(hdc, &rect, CreateSolidBrush(RGB(80, 80, 80)));

        hPen = CreatePen(1, 4, RGB(255, 255, 255));
        SelectObject(hdc, hPen);

        Line(hdc, 0, 200, 0, 500);//ось T
        Line(hdc, 0, 200, 800 * sqrt(3) / 2, -800 / sqrt(3) + 200);//ось y
        Line(hdc, 0, 200, -800 * sqrt(3) / 2, -800 / sqrt(3) + 200);//ось x

        //заходим критическую секцию (обращаемся к глобальным данным А0)
        EnterCriticalSection(&cs);
        for (int i = 0; i < 21; i++)
            for (int j = 0; j < 21; j++)
                A[i][j] = A0[i][j];
        //покидаем критическую секцию
        LeaveCriticalSection(&cs);

        int* e1;
        int* e2;

        for (int i = 0; i < 21; i++)
        {
            for (int j = 0; j < 20; j++)
            {
                e1 = Preobr(i * hx, j * hy, A[j][i]);
                e2 = Preobr(i * hx, j * hy + hy, A[j + 1][i]);

                Line(hdc, e1[0], e1[1] + 200, e2[0], e2[1] + 200);
            }
        }

        for (int j = 0; j < 21; j++)
        {
            for (int i = 0; i < 20; i++)
            {
                e1 = Preobr(i * hx, j * hy, A[j][i]);
                e2 = Preobr(i * hx + hx, j * hy, A[j][i + 1]);

                Line(hdc, e1[0], e1[1] + 200, e2[0], e2[1] + 200);
            }
        }
        return(0);

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK Input(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            GetDlgItemText(hDlg, IDC_EDIT1, bufC1, 5);
            GetDlgItemText(hDlg, IDC_EDIT2, bufC2, 5);

            C1 = _wtoi(bufC1);
            C2 = _wtoi(bufC2);

            EndDialog(hDlg, 1);
            break;
        case IDFILE:
            std::ifstream file;
            file.open("input.txt");

            file >> C1;
            file >> C2;
            EndDialog(hDlg, 1);
            break;
        }
        return (INT_PTR)TRUE;
    }
    return (INT_PTR)TRUE;
}

BOOL Line(HDC hdc, int x1, int y1, int x2, int y2)
{
    MoveToEx(hdc, x1, y1, NULL); //сделать текущими координаты x1, y1
    return LineTo(hdc, x2, y2); //нарисовать линию
}

void NextLawer(int t) {

    double A[21][21];

    for (int i = 0; i < 21; i++) {
        A[0][i] = u0x();
        A[20][i] = ubx();
    }

    for (int j = 0; j < 21; j++) {
        A[j][0] = u0y(j * hy, t * ht);
        A[j][20] = uay(j * hy, t * ht);
    }

    for (int j = 1; j < 20; j++)
        for (int i = 1; i < 20; i++)
            A[j][i] = ht / pow(hx, 2) * (U[j][i + 1] - 2 * U[j][i] + U[j][i - 1]) + ht / pow(hy, 2) * (U[j + 1][i] - 2 * U[j][i] + U[j - 1][i]) + U[j][i] + ht * f(i * hx, j * hy, t * ht);

    for (int j = 0; j < 21; j++)
        for (int i = 0; i < 21; i++)
            U[i][j] = A[i][j];
}

void Thread(PVOID lpParam) {
    WaitForSingleObject(hEvent, INFINITE);

    for (int i = 0; i < 21; i++) {
        U[0][i] = u0x();
        U[20][i] = ubx();
    }

    for (int j = 0; j < 21; j++) {
        U[j][0] = u0y(j * hy, 0);
        U[j][20] = uay(j * hy, 0);
    }

    for (int j = 1; j < 20; j++)
        for (int i = 1; i < 20; i++)
            U[j][i] = v();

    int k = 0;
    t = 0;
    while (t < T)
    {
        NextLawer(t);
        if (k > K)
        {
            EnterCriticalSection(&cs);
            for (int j = 0; j < 21; j++)
                for (int i = 0; i < 21; i++)
                    A0[i][j] = U[i][j];
            LeaveCriticalSection(&cs);

            SendMessage(hwnd, WM_CALC_FINISHED, 0, 0);

            k = 0;
        }
        k++;
        t++;
    }

    _endthread();
}

int* Preobr(double x, double y, double z) {
    int* e = new int[2];

    x *= 30 / hx;
    y *= 30 / hy;
    z *= 30 / C1;
    e[0] = sqrt(3) / 2 * (y - x);
    e[1] = -1 / sqrt(3) * (x + y) + z;

    return e;
}
