#include "Visualizer.h"

#ifdef _WIN32
#include <windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

namespace {
    // Current account snapshot for painting
    struct ChartData {
        std::string accNo;
        std::string holder;
        std::string type;
        double balance;
        double minBalance;
        double dailyLimit;
        double withdrawnToday;
        double remainingLimit;
        struct TxnItem {
            std::string id;
            std::string type;
            double amount;
            double balanceAfter;
            bool isDebit;
        };
        std::vector<TxnItem> transactions;
    };

    ChartData g_data;

    void DrawBankIcon(HDC hdc, int x, int y, int size) {
        // Draw Classical Bank Building Pediment and Pillars using GDI primitives
        HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
        HBRUSH hBrush = CreateSolidBrush(RGB(56, 189, 248)); // Cyan accent
        HGDIOBJ oldPen = SelectObject(hdc, hPen);
        HGDIOBJ oldBrush = SelectObject(hdc, hBrush);

        // Triangular Pediment / Roof
        POINT roof[3] = {
            { x + size / 2, y },
            { x, y + size / 3 },
            { x + size, y + size / 3 }
        };
        Polygon(hdc, roof, 3);

        // Architrave (beam)
        Rectangle(hdc, x + 2, y + size / 3, x + size - 2, y + size / 3 + 4);

        // 4 Pillars
        int pWidth = (size - 16) / 4;
        for (int i = 0; i < 4; ++i) {
            int px = x + 4 + i * (pWidth + 3);
            Rectangle(hdc, px, y + size / 3 + 4, px + pWidth, y + (size * 4) / 5);
        }

        // Base Podium
        Rectangle(hdc, x - 2, y + (size * 4) / 5, x + size + 2, y + size);

        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(hPen);
        DeleteObject(hBrush);
    }

    void PaintDashboard(HDC hdc, int width, int height) {
        // Double buffering: Create memory DC
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
        HGDIOBJ oldBmp = SelectObject(memDC, memBitmap);

        // 1. Fill Background
        HBRUSH bgBrush = CreateSolidBrush(RGB(248, 250, 252)); // Light slate #f8fafc
        RECT fullRect = { 0, 0, width, height };
        FillRect(memDC, &fullRect, bgBrush);
        DeleteObject(bgBrush);

        // 2. Header Banner (Dark Navy)
        RECT headerRect = { 0, 0, width, 90 };
        HBRUSH headerBrush = CreateSolidBrush(RGB(15, 23, 42)); // #0f172a
        FillRect(memDC, &headerRect, headerBrush);
        DeleteObject(headerBrush);

        // Draw Bank Icon in Header
        DrawBankIcon(memDC, 25, 20, 50);

        // Header Text
        SetBkMode(memDC, TRANSPARENT);
        HFONT hTitleFont = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                      CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
        HGDIOBJ oldFont = SelectObject(memDC, hTitleFont);
        SetTextColor(memDC, RGB(255, 255, 255));
        TextOutA(memDC, 90, 18, "APEX NATIONAL BANK - VISUAL ANALYTICS", 37);

        HFONT hSubFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                     DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                     CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
        SelectObject(memDC, hSubFont);
        SetTextColor(memDC, RGB(148, 163, 184)); // #94a3b8
        TextOutA(memDC, 90, 48, "Computer Graphics & Object-Oriented Financial Dashboard", 55);

        // 3. Account Overview Card (Rounded Rectangle)
        HPEN cardPen = CreatePen(PS_SOLID, 1, RGB(226, 232, 240));
        HBRUSH cardBrush = CreateSolidBrush(RGB(255, 255, 255));
        SelectObject(memDC, cardPen);
        SelectObject(memDC, cardBrush);
        RoundRect(memDC, 25, 105, width - 25, 215, 12, 12);

        // Card Content
        HFONT hCardLabelFont = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
        HFONT hCardBoldFont = CreateFontA(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                          CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
        HFONT hBalFont = CreateFontA(26, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                     DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                     CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

        SelectObject(memDC, hCardLabelFont);
        SetTextColor(memDC, RGB(100, 116, 139));
        TextOutA(memDC, 45, 118, "ACCOUNT HOLDER", 14);
        SelectObject(memDC, hCardBoldFont);
        SetTextColor(memDC, RGB(15, 23, 42));
        TextOutA(memDC, 45, 138, g_data.holder.c_str(), (int)g_data.holder.length());

        SelectObject(memDC, hCardLabelFont);
        SetTextColor(memDC, RGB(100, 116, 139));
        TextOutA(memDC, 260, 118, "ACCOUNT NUMBER", 14);
        SelectObject(memDC, hCardBoldFont);
        SetTextColor(memDC, RGB(2, 132, 199)); // #0284c7
        TextOutA(memDC, 260, 138, g_data.accNo.c_str(), (int)g_data.accNo.length());

        SelectObject(memDC, hCardLabelFont);
        SetTextColor(memDC, RGB(100, 116, 139));
        TextOutA(memDC, 440, 118, "ACCOUNT TYPE", 12);
        SelectObject(memDC, hCardBoldFont);
        SetTextColor(memDC, RGB(15, 23, 42));
        std::string typeBadge = g_data.type + " ACCOUNT";
        TextOutA(memDC, 440, 138, typeBadge.c_str(), (int)typeBadge.length());

        SelectObject(memDC, hCardLabelFont);
        SetTextColor(memDC, RGB(100, 116, 139));
        TextOutA(memDC, 650, 118, "CURRENT BALANCE", 15);
        SelectObject(memDC, hBalFont);
        SetTextColor(memDC, RGB(16, 185, 129)); // Emerald #10b981
        std::ostringstream balOss;
        balOss << "INR " << std::fixed << std::setprecision(2) << g_data.balance;
        std::string balStr = balOss.str();
        TextOutA(memDC, 650, 135, balStr.c_str(), (int)balStr.length());

        // 4. Daily Withdrawal Limit Gauge (Progress Bar) - Requirement d
        SelectObject(memDC, hCardLabelFont);
        SetTextColor(memDC, RGB(71, 85, 105));
        std::string meterTitle = "Daily Withdrawal Allowance Gauge:";
        TextOutA(memDC, 45, 175, meterTitle.c_str(), (int)meterTitle.length());

        int meterX = 260;
        int meterY = 175;
        int meterW = 360;
        int meterH = 20;

        // Meter Track (Background)
        HBRUSH trackBrush = CreateSolidBrush(RGB(226, 232, 240));
        HPEN trackPen = CreatePen(PS_SOLID, 1, RGB(203, 213, 225));
        SelectObject(memDC, trackBrush);
        SelectObject(memDC, trackPen);
        RoundRect(memDC, meterX, meterY, meterX + meterW, meterY + meterH, 6, 6);
        DeleteObject(trackBrush);
        DeleteObject(trackPen);

        // Meter Fill
        double ratio = (g_data.dailyLimit > 0.0) ? (g_data.withdrawnToday / g_data.dailyLimit) : 0.0;
        if (ratio > 1.0) ratio = 1.0;
        int fillW = (int)(ratio * meterW);

        COLORREF fillColor = RGB(34, 197, 94); // Green
        if (ratio >= 0.85) {
            fillColor = RGB(239, 68, 68); // Red
        } else if (ratio >= 0.50) {
            fillColor = RGB(245, 158, 11); // Amber
        }

        if (fillW > 0) {
            HBRUSH fillBrush = CreateSolidBrush(fillColor);
            HPEN fillPen = CreatePen(PS_NULL, 0, RGB(0,0,0));
            SelectObject(memDC, fillBrush);
            SelectObject(memDC, fillPen);
            RoundRect(memDC, meterX, meterY, meterX + fillW, meterY + meterH, 6, 6);
            DeleteObject(fillBrush);
            DeleteObject(fillPen);
        }

        // Limit stats text
        std::ostringstream limitOss;
        limitOss << "Used: INR " << std::fixed << std::setprecision(0) << g_data.withdrawnToday
                 << " / " << g_data.dailyLimit
                 << " (" << (int)(ratio * 100) << "%) | Remaining: INR " << g_data.remainingLimit;
        std::string limitStr = limitOss.str();
        SelectObject(memDC, hCardLabelFont);
        SetTextColor(memDC, RGB(51, 65, 85));
        TextOutA(memDC, meterX + meterW + 15, 177, limitStr.c_str(), (int)limitStr.length());

        DeleteObject(cardBrush);
        DeleteObject(cardPen);

        // 5. 2D Computer Graphics Chart Area (Bar Chart + Line Chart)
        int chartLeft = 55;
        int chartTop = 235;
        int chartRight = width - 40;
        int chartBottom = height - 85;

        // Chart Card container
        HBRUSH chartBg = CreateSolidBrush(RGB(255, 255, 255));
        HPEN chartBorder = CreatePen(PS_SOLID, 1, RGB(226, 232, 240));
        SelectObject(memDC, chartBg);
        SelectObject(memDC, chartBorder);
        RoundRect(memDC, 25, 225, width - 25, height - 55, 12, 12);
        DeleteObject(chartBg);
        DeleteObject(chartBorder);

        // Chart Title & Subtitle
        SelectObject(memDC, hCardBoldFont);
        SetTextColor(memDC, RGB(15, 23, 42));
        TextOutA(memDC, 45, 238, "Transaction History & Balance Trend (Computer Graphics View)", 61);

        // Legend
        int legX = chartRight - 360;
        int legY = 242;

        // Green square (Deposit)
        HBRUSH depBrush = CreateSolidBrush(RGB(16, 185, 129));
        RECT rDep = { legX, legY + 2, legX + 12, legY + 14 };
        FillRect(memDC, &rDep, depBrush);
        DeleteObject(depBrush);
        SelectObject(memDC, hCardLabelFont);
        SetTextColor(memDC, RGB(71, 85, 105));
        TextOutA(memDC, legX + 16, legY, "Deposit (+)", 11);

        // Red square (Withdrawal)
        HBRUSH wthBrush = CreateSolidBrush(RGB(239, 68, 68));
        RECT rWth = { legX + 105, legY + 2, legX + 117, legY + 14 };
        FillRect(memDC, &rWth, wthBrush);
        DeleteObject(wthBrush);
        TextOutA(memDC, legX + 121, legY, "Withdrawal (-)", 14);

        // Blue line (Balance)
        HPEN bLinePen = CreatePen(PS_SOLID, 2, RGB(59, 130, 246));
        SelectObject(memDC, bLinePen);
        MoveToEx(memDC, legX + 230, legY + 8, NULL);
        LineTo(memDC, legX + 250, legY + 8);
        DeleteObject(bLinePen);
        TextOutA(memDC, legX + 255, legY, "Balance Curve", 13);

        // Plot Coordinates
        int plotX0 = chartLeft + 45;
        int plotY0 = chartBottom - 35;
        int plotX1 = chartRight - 30;
        int plotY1 = chartTop + 55;

        // Grid & Axes
        HPEN gridPen = CreatePen(PS_DOT, 1, RGB(226, 232, 240));
        HPEN axisPen = CreatePen(PS_SOLID, 2, RGB(148, 163, 184));
        SelectObject(memDC, gridPen);

        // Max amount for scaling
        double maxVal = 10000.0;
        for (const auto& t : g_data.transactions) {
            if (t.amount > maxVal) maxVal = t.amount;
            if (t.balanceAfter > maxVal) maxVal = t.balanceAfter;
        }
        // Round maxVal up to nearest 5000 or 10000
        maxVal = std::ceil(maxVal / 5000.0) * 5000.0;
        if (maxVal <= 0) maxVal = 10000.0;

        // Draw 5 horizontal grid lines
        for (int i = 0; i <= 5; ++i) {
            int yPos = plotY0 - (int)((double)i / 5.0 * (plotY0 - plotY1));
            SelectObject(memDC, gridPen);
            MoveToEx(memDC, plotX0, yPos, NULL);
            LineTo(memDC, plotX1, yPos);

            // Label
            double gridVal = (maxVal / 5.0) * i;
            std::ostringstream gOss;
            gOss << (int)gridVal;
            std::string gStr = gOss.str();
            SelectObject(memDC, hCardLabelFont);
            SetTextColor(memDC, RGB(148, 163, 184));
            TextOutA(memDC, chartLeft - 10, yPos - 7, gStr.c_str(), (int)gStr.length());
        }
        DeleteObject(gridPen);

        // Draw Axes
        SelectObject(memDC, axisPen);
        MoveToEx(memDC, plotX0, plotY1 - 10, NULL);
        LineTo(memDC, plotX0, plotY0);
        LineTo(memDC, plotX1 + 10, plotY0);
        DeleteObject(axisPen);

        // Draw transactions bars & balance curve
        size_t count = g_data.transactions.size();
        if (count > 0) {
            // Take at most 10 recent transactions
            size_t startIdx = (count > 10) ? (count - 10) : 0;
            size_t plotCount = count - startIdx;
            int stepX = (plotX1 - plotX0) / (plotCount + 1);
            int barWidth = std::max(18, stepX / 3);

            std::vector<POINT> balancePoints;

            for (size_t i = 0; i < plotCount; ++i) {
                const auto& txn = g_data.transactions[startIdx + i];
                int cx = plotX0 + (int)(i + 1) * stepX;

                // Bar height
                double amtRatio = txn.amount / maxVal;
                int barH = (int)(amtRatio * (plotY0 - plotY1));
                if (barH < 3) barH = 3;

                COLORREF barColor = txn.isDebit ? RGB(239, 68, 68) : RGB(16, 185, 129);
                HBRUSH bBrush = CreateSolidBrush(barColor);
                HPEN bPen = CreatePen(PS_SOLID, 1, barColor);
                SelectObject(memDC, bBrush);
                SelectObject(memDC, bPen);

                RoundRect(memDC, cx - barWidth / 2, plotY0 - barH,
                          cx + barWidth / 2, plotY0, 4, 4);

                DeleteObject(bBrush);
                DeleteObject(bPen);

                // Transaction label below X axis
                SelectObject(memDC, hCardLabelFont);
                SetTextColor(memDC, RGB(71, 85, 105));
                TextOutA(memDC, cx - 18, plotY0 + 6, txn.id.c_str(), (int)txn.id.length());

                // Amount label above bar
                std::ostringstream aOss;
                aOss << (int)txn.amount;
                std::string aStr = aOss.str();
                SetTextColor(memDC, barColor);
                TextOutA(memDC, cx - 12, plotY0 - barH - 16, aStr.c_str(), (int)aStr.length());

                // Calculate balance point for line chart
                double balRatio = txn.balanceAfter / maxVal;
                int balY = plotY0 - (int)(balRatio * (plotY0 - plotY1));
                balancePoints.push_back({ cx, balY });
            }

            // Draw Balance Trend Polyline
            if (balancePoints.size() >= 2) {
                HPEN trendPen = CreatePen(PS_SOLID, 3, RGB(59, 130, 246));
                SelectObject(memDC, trendPen);
                Polyline(memDC, balancePoints.data(), (int)balancePoints.size());
                DeleteObject(trendPen);
            }

            // Draw marker dots on balance points
            HBRUSH dotBrush = CreateSolidBrush(RGB(59, 130, 246));
            HPEN dotPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
            SelectObject(memDC, dotBrush);
            SelectObject(memDC, dotPen);
            for (const auto& pt : balancePoints) {
                Ellipse(memDC, pt.x - 5, pt.y - 5, pt.x + 5, pt.y + 5);
            }
            DeleteObject(dotBrush);
            DeleteObject(dotPen);
        }

        // 6. Footer instructions
        SetBkMode(memDC, TRANSPARENT);
        SelectObject(memDC, hCardLabelFont);
        SetTextColor(memDC, RGB(100, 116, 139));
        TextOutA(memDC, 35, height - 42, "Tip: Press ESC or close this window to return to the interactive console terminal.", 82);

        // Clean up fonts
        DeleteObject(hTitleFont);
        DeleteObject(hSubFont);
        DeleteObject(hCardLabelFont);
        DeleteObject(hCardBoldFont);
        DeleteObject(hBalFont);

        // Blit memory DC to window DC
        BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

        SelectObject(memDC, oldBmp);
        DeleteObject(memBitmap);
        DeleteDC(memDC);
    }

    LRESULT CALLBACK DashboardWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
            case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                RECT rc;
                GetClientRect(hwnd, &rc);
                PaintDashboard(hdc, rc.right - rc.left, rc.bottom - rc.top);
                EndPaint(hwnd, &ps);
                return 0;
            }
            case WM_ERASEBKGND:
                return 1; // Handled in double buffer
            case WM_KEYDOWN:
                if (wParam == VK_ESCAPE) {
                    DestroyWindow(hwnd);
                    return 0;
                }
                break;
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

bool Visualizer::isSupported() {
    return true; // Native Windows Win32 GDI is supported on Windows
}

void Visualizer::renderAccountDashboard(const Account& account) {
    // Populate snapshot data
    g_data.accNo = account.getAccountNumber();
    g_data.holder = account.getHolderName();
    g_data.type = account.getAccountType();
    g_data.balance = account.getRawBalance();
    g_data.minBalance = account.getMinimumBalance();
    g_data.dailyLimit = account.getDailyWithdrawalLimit();
    g_data.withdrawnToday = account.getWithdrawnToday();
    g_data.remainingLimit = const_cast<Account&>(account).getRemainingDailyLimit();

    g_data.transactions.clear();
    for (const auto& t : account.getTransactions()) {
        ChartData::TxnItem item;
        item.id = t.getTxnId();
        item.type = t.getTypeString();
        item.amount = t.getAmount();
        item.balanceAfter = t.getBalanceAfter();
        item.isDebit = (t.getType() == TransactionType::WITHDRAWAL);
        g_data.transactions.push_back(item);
    }

    const char CLASS_NAME[] = "OOP_CG_BankDashboardClass";

    WNDCLASSA wc = {};
    wc.lpfnWndProc = DashboardWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassA(&wc);

    int winWidth = 960;
    int winHeight = 690;
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int posX = std::max(20, (screenW - winWidth) / 2);
    int posY = std::max(20, (screenH - winHeight) / 2);

    std::string title = "Bank Visual Analytics | Account: " + g_data.accNo + " (" + g_data.holder + ")";

    HWND hwnd = CreateWindowExA(
        0,
        CLASS_NAME,
        title.c_str(),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        posX, posY, winWidth, winHeight,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    if (hwnd == NULL) {
        return;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Modal event loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnregisterClassA(CLASS_NAME, GetModuleHandle(NULL));
}

#else

#include <iostream>

bool Visualizer::isSupported() {
    return false;
}

void Visualizer::renderAccountDashboard(const Account& account) {
    std::cout << "\n==============================================================\n";
    std::cout << "           COMPUTER GRAPHICS (CG) VISUAL ANALYTICS            \n";
    std::cout << "==============================================================\n";
    std::cout << "[NOTE] The 2D Graphical Dashboard is available on Windows only (Win32 GDI).\n";
    std::cout << "       Current Account: " << account.getAccountNumber() << " (" << account.getHolderName() << ")\n";
    std::cout << "       Tip: You can view rich 2D graphics in your web browser by running:\n";
    std::cout << "            python3 web_app.py -> http://localhost:5000\n";
    std::cout << "==============================================================\n\n";
}

#endif
