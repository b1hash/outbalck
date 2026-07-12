#pragma once

#include <string>

// Embedded HTML page for web remote control
// Split into parts to avoid MSVC string literal limit (16KB)

inline std::string GetWebPageHTML() {
    std::string html;

    // Part 1: Head and base styles
    html += R"HTML(<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, viewport-fit=cover">
    <title>SimpleRemoter</title>
    <!-- PWA / iOS Standalone App Support -->
    <meta name="apple-mobile-web-app-capable" content="yes">
    <meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
    <meta name="apple-mobile-web-app-title" content="Remoter">
    <meta name="mobile-web-app-capable" content="yes">
    <meta name="theme-color" content="#1a1a2e">
    <link rel="manifest" href="/manifest.json">
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
            color: #eee;
            min-height: 100vh;
            overflow-x: hidden;
        }
        .page { display: none !important; padding: 20px; min-height: 100vh; }
        .page.active { display: block !important; }
        #login-page.active {
            display: flex !important;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            background: radial-gradient(ellipse at center, #1a1a2e 0%, #0f0f1a 100%);
        }
        .login-form {
            background: rgba(22, 33, 62, 0.95);
            padding: 40px;
            border-radius: 16px;
            width: 100%;
            max-width: 360px;
            box-shadow: 0 8px 32px rgba(0,0,0,0.4);
            backdrop-filter: blur(10px);
            border: 1px solid rgba(233, 69, 96, 0.2);
        }
        .login-form h1 {
            text-align: center;
            margin-bottom: 30px;
            color: #e94560;
            font-size: 28px;
            text-shadow: 0 0 20px rgba(233, 69, 96, 0.3);
        }
        .login-form input {
            width: 100%;
            padding: 14px 16px;
            margin-bottom: 16px;
            border: 1px solid rgba(255,255,255,0.1);
            border-radius: 10px;
            background: rgba(15, 52, 96, 0.8);
            color: #fff;
            font-size: 16px;
            outline: none;
            transition: all 0.3s;
        }
        .login-form input:focus {
            border-color: #e94560;
            box-shadow: 0 0 0 3px rgba(233, 69, 96, 0.2);
        }
        .login-form input::placeholder { color: #666; }
        .login-form button {
            width: 100%;
            padding: 14px;
            border: none;
            border-radius: 10px;
            background: linear-gradient(135deg, #e94560 0%, #c73e54 100%);
            color: #fff;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        .login-form button:hover { transform: translateY(-2px); box-shadow: 0 4px 15px rgba(233, 69, 96, 0.4); }
        .login-form button:disabled { background: #444; cursor: not-allowed; transform: none; box-shadow: none; }
        .error-msg { color: #e94560; text-align: center; margin-top: 16px; font-size: 14px; }
        .conn-status { text-align: center; margin-bottom: 20px; font-size: 13px; color: #666; }
        .conn-status.connected { color: #4caf50; }
        .conn-status.disconnected { color: #f44336; }
)HTML";

    // Part 2: Device page styles
    html += R"HTML(
        #devices-page { max-width: 1400px; margin: 0 auto; }
        .page-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 24px;
            flex-wrap: wrap;
            gap: 16px;
        }
        .page-header h2 {
            color: #e94560;
            font-size: 24px;
            display: flex;
            align-items: center;
            gap: 10px;
        }
        .device-count {
            background: rgba(233, 69, 96, 0.2);
            color: #e94560;
            padding: 4px 12px;
            border-radius: 20px;
            font-size: 14px;
            font-weight: normal;
        }
        .toolbar {
            display: flex;
            gap: 12px;
            align-items: center;
            flex-wrap: wrap;
        }
        .search-box {
            position: relative;
            flex: 1;
            min-width: 180px;
            max-width: 300px;
        }
        .search-box input {
            width: 100%;
            padding: 10px 16px;
            border: 1px solid rgba(255,255,255,0.1);
            border-radius: 8px;
            background: rgba(15, 52, 96, 0.6);
            color: #fff;
            font-size: 14px;
            outline: none;
            transition: all 0.3s;
        }
        .search-box input:focus {
            border-color: #e94560;
            background: rgba(15, 52, 96, 0.9);
        }
        .search-box input::placeholder { color: #666; }
        .search-box input { padding-right: 32px; }
        .search-clear {
            position: absolute;
            right: 8px;
            top: 50%;
            transform: translateY(-50%);
            background: none;
            border: none;
            color: #666;
            font-size: 16px;
            cursor: pointer;
            padding: 4px 8px;
            display: none;
        }
        .search-clear:hover { color: #e94560; }
        .view-toggle {
            display: flex;
            border: 1px solid rgba(255,255,255,0.1);
            border-radius: 8px;
            overflow: hidden;
        }
        .view-btn {
            padding: 8px 14px;
            border: none;
            background: rgba(15, 52, 96, 0.6);
            color: #888;
            font-size: 13px;
            cursor: pointer;
            transition: all 0.2s;
        }
        .view-btn:first-child { border-right: 1px solid rgba(255,255,255,0.1); }
        .view-btn:hover { color: #fff; }
        .view-btn.active { background: rgba(233, 69, 96, 0.3); color: #e94560; }
        .refresh-btn {
            padding: 10px 20px;
            border: none;
            border-radius: 8px;
            background: linear-gradient(135deg, #0f3460 0%, #1a4a7a 100%);
            color: #fff;
            font-size: 14px;
            cursor: pointer;
            transition: all 0.3s;
        }
        .refresh-btn:hover { transform: translateY(-1px); box-shadow: 0 4px 12px rgba(15, 52, 96, 0.4); }
        .logout-btn {
            padding: 10px 20px;
            border: none;
            border-radius: 8px;
            background: linear-gradient(135deg, #c0392b 0%, #e74c3c 100%);
            color: #fff;
            font-size: 14px;
            cursor: pointer;
            transition: all 0.3s;
            margin-left: 10px;
        }
        .logout-btn:hover { transform: translateY(-1px); box-shadow: 0 4px 12px rgba(192, 57, 43, 0.4); }
)HTML";

    // Part 3: Device card styles
    html += R"HTML(
        .device-grid {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(340px, 1fr));
            gap: 16px;
            margin-bottom: 24px;
        }
        .device-card {
            background: rgba(22, 33, 62, 0.8);
            padding: 20px;
            border-radius: 12px;
            cursor: pointer;
            transition: all 0.3s;
            border: 1px solid rgba(255,255,255,0.05);
            position: relative;
            overflow: hidden;
        }
        .device-card::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            height: 3px;
            background: linear-gradient(90deg, #e94560, #0f3460);
            opacity: 0;
            transition: opacity 0.3s;
        }
        .device-card:hover {
            transform: translateY(-4px);
            box-shadow: 0 8px 25px rgba(0,0,0,0.3);
            border-color: rgba(233, 69, 96, 0.3);
        }
        .device-card:hover::before { opacity: 1; }
        .device-card h3 {
            color: #fff;
            font-size: 16px;
            margin-bottom: 12px;
            white-space: nowrap;
            overflow: hidden;
            text-overflow: ellipsis;
        }
        .device-card .info {
            color: #888;
            font-size: 13px;
            margin-bottom: 6px;
            display: flex;
            align-items: center;
            gap: 8px;
        }
        .device-card .info-label { color: #666; min-width: 32px; }
        .device-card .resolution { color: #666; font-size: 12px; }
        .device-card .info-row { display: flex; gap: 16px; flex-wrap: wrap; }
        .device-card .active-window {
            color: #888; font-size: 12px; margin-top: 8px;
            white-space: nowrap; overflow: hidden; text-overflow: ellipsis;
            max-width: 100%; opacity: 0.8;
        }
        .device-card .meta-row { display: flex; gap: 12px; margin-top: 6px; font-size: 12px; color: #666; }
        .device-card .meta-item { display: flex; align-items: center; gap: 4px; }
        .device-card .meta-item.rtt { font-weight: 500; }
        .device-card .meta-item.rtt.rtt-good { color: #4caf50; }
        .device-card .meta-item.rtt.rtt-medium { color: #ff9800; }
        .device-card .meta-item.rtt.rtt-poor { color: #f44336; }
        .no-devices {
            text-align: center;
            color: #666;
            padding: 60px 20px;
            background: rgba(22, 33, 62, 0.5);
            border-radius: 12px;
            border: 1px dashed rgba(255,255,255,0.1);
        }
        .no-devices h3 { color: #888; margin-bottom: 10px; }
        /* List view styles */
        .device-grid.list-view {
            display: flex;
            flex-direction: column;
            gap: 8px;
        }
        .device-grid.list-view .device-card {
            display: flex;
            align-items: center;
            padding: 12px 16px;
            gap: 20px;
        }
        .device-grid.list-view .device-card::before { display: none; }
        .device-grid.list-view .device-card h3 {
            min-width: 180px;
            margin-bottom: 0;
            font-size: 14px;
        }
        .device-grid.list-view .device-card .info {
            margin-bottom: 0;
            min-width: 120px;
        }
        .device-grid.list-view .device-card .info-row { flex-wrap: nowrap; gap: 12px; }
        .device-grid.list-view .device-card .meta-row { margin-top: 0; }
        .device-grid.list-view .device-card .active-window { 
            margin-top: 0; max-width: 200px; flex-shrink: 1;
        }
)HTML";

    // Part 4: Pagination styles
    html += R"HTML(
        .pagination {
            display: flex;
            justify-content: center;
            align-items: center;
            gap: 8px;
            padding: 20px 0;
        }
        .pagination button {
            padding: 8px 14px;
            border: 1px solid rgba(255,255,255,0.1);
            border-radius: 6px;
            background: rgba(15, 52, 96, 0.6);
            color: #aaa;
            font-size: 14px;
            cursor: pointer;
            transition: all 0.2s;
        }
        .pagination button:hover:not(:disabled) {
            border-color: #e94560;
            color: #fff;
        }
        .pagination button:disabled {
            opacity: 0.4;
            cursor: not-allowed;
        }
        .pagination button.active {
            background: #e94560;
            border-color: #e94560;
            color: #fff;
        }
        .pagination .page-info {
            color: #666;
            font-size: 13px;
            padding: 0 12px;
        }
        .stats-bar {
            display: flex;
            gap: 20px;
            margin-bottom: 20px;
            padding: 12px 16px;
            background: rgba(15, 52, 96, 0.4);
            border-radius: 8px;
            font-size: 13px;
        }
        .stats-bar .stat { color: #888; }
        .stats-bar .stat strong { color: #e94560; }
)HTML";

    // Part 5: Screen page styles
    html += R"HTML(
        #screen-page {
            position: fixed;
            top: 0; left: 0; right: 0; bottom: 0;
            background: #000;
            padding: 0;
        }
        #screen-page.active { display: flex !important; flex-direction: column; }
        .canvas-container { flex: 1; display: flex; align-items: center; justify-content: center; overflow: hidden; background: #111; position: relative; }
        #screen-canvas { max-width: 100%; max-height: 100%; }
        .screen-toolbar {
            background: linear-gradient(180deg, rgba(0,0,0,0.95) 0%, rgba(0,0,0,0.85) 100%);
            padding: 12px 20px;
            display: flex;
            justify-content: space-between;
            align-items: center;
            flex-shrink: 0;
            border-bottom: 1px solid rgba(255,255,255,0.1);
        }
        .back-btn {
            background: rgba(255,255,255,0.1);
            border: none;
            color: #fff;
            font-size: 14px;
            cursor: pointer;
            padding: 8px 16px;
            border-radius: 6px;
            transition: all 0.2s;
        }
        .back-btn:hover { background: #e94560; }
        .toolbar-info { text-align: center; flex: 1; }
        .toolbar-info .device-name { font-weight: 600; font-size: 15px; color: #fff; }
        .toolbar-info .conn-info { font-size: 12px; color: #888; margin-top: 4px; }
        .screen-status { font-size: 12px; padding: 6px 14px; border-radius: 20px; font-weight: 500; }
        .screen-status.connecting { background: rgba(255, 152, 0, 0.2); color: #ff9800; }
        .screen-status.connected { background: rgba(76, 175, 80, 0.2); color: #4caf50; }
        .screen-status.error { background: rgba(244, 67, 54, 0.2); color: #f44336; }
        .toolbar-right { display: flex; align-items: center; gap: 12px; }
        .fullscreen-btn {
            background: rgba(255,255,255,0.1);
            border: none;
            color: #fff;
            font-size: 18px;
            cursor: pointer;
            padding: 6px 10px;
            border-radius: 6px;
            transition: all 0.2s;
            line-height: 1;
        }
        .fullscreen-btn:hover { background: rgba(255,255,255,0.2); }
        .toolbar-btn-bar {
            background: rgba(255,255,255,0.1);
            border: none;
            color: #fff;
            font-size: 18px;
            cursor: pointer;
            padding: 6px 10px;
            border-radius: 6px;
            transition: all 0.2s;
            line-height: 1;
        }
        .toolbar-btn-bar:hover { background: rgba(255,255,255,0.2); }
        .toolbar-btn-bar.active { background: rgba(52,199,89,0.8); }
        .toolbar-btn-bar.active:hover { background: rgba(52,199,89,1); }
        .toolbar-btn-bar:disabled { opacity: 0.4; cursor: not-allowed; }
        .toolbar-btn-bar:disabled:hover { background: rgba(255,255,255,0.1); }
        #screen-page:fullscreen .screen-toolbar { display: none; }
        #screen-page:-webkit-full-screen .screen-toolbar { display: none; }
        #screen-page:fullscreen .canvas-container { height: 100vh; }
        #screen-page:-webkit-full-screen .canvas-container { height: 100vh; }
        /* iOS pseudo-fullscreen (no native fullscreen API support) */
        #screen-page.pseudo-fullscreen {
            position: fixed !important;
            top: 0 !important; left: 0 !important; right: 0 !important; bottom: 0 !important;
            width: 100vw !important; height: 100vh !important;
            height: 100dvh !important; /* Dynamic viewport for iOS */
            z-index: 99999;
            background: #000;
            padding: 0 !important;
            margin: 0 !important;
        }
        #screen-page.pseudo-fullscreen .screen-toolbar { display: none !important; }
        #screen-page.pseudo-fullscreen .canvas-container {
            width: 100vw !important; height: 100vh !important;
            height: 100dvh !important;
            max-height: none !important;
        }
        #screen-page.pseudo-fullscreen #screen-canvas {
            max-width: 100vw !important; max-height: 100vh !important;
            max-height: 100dvh !important;
            object-fit: contain;
        }
        #screen-page.pseudo-fullscreen .toolbar-toggle {
            display: flex; /* Show toolbar toggle in fullscreen */
        }
        #screen-page:fullscreen .toolbar-toggle,
        #screen-page:-webkit-full-screen .toolbar-toggle {
            display: flex;
        }
        .compat-warning {
            background: linear-gradient(90deg, #ff9800, #f57c00);
            color: #000;
            padding: 12px;
            text-align: center;
            font-size: 14px;
            font-weight: 500;
        }
        /* Mobile floating controls */
        /* Floating toolbar menu - minimal icon style */
        .floating-toolbar {
            position: fixed;
            top: 8px;
            left: 50%;
            transform: translateX(-50%);
            z-index: 1001;
            display: none;
            background: rgba(0,0,0,0.75);
            border-radius: 22px;
            padding: 4px 8px;
            gap: 4px;
            box-shadow: 0 2px 12px rgba(0,0,0,0.4);
        }
        .floating-toolbar.visible { display: flex; align-items: center; }
        .toolbar-btn {
            background: transparent;
            border: none;
            color: #fff;
            font-size: 20px;
            cursor: pointer;
            width: 40px;
            height: 40px;
            border-radius: 50%;
            transition: all 0.15s;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .toolbar-btn:hover { background: rgba(255,255,255,0.2); }
        .toolbar-btn:active { transform: scale(0.9); background: rgba(255,255,255,0.3); }
        .toolbar-btn.active { background: rgba(52,199,89,0.8); }
        .toolbar-btn.active:hover { background: rgba(52,199,89,1); }
        .toolbar-btn:disabled { opacity: 0.3; cursor: not-allowed; }
        .toolbar-btn:disabled:hover { background: transparent; }
        .toolbar-btn:disabled:active { transform: none; }
        .toolbar-toggle {
            position: fixed;
            top: 8px;
            left: 50%;
            transform: translateX(-50%);
            z-index: 1000;
            background: rgba(0,0,0,0.5);
            border: none;
            color: #fff;
            font-size: 16px;
            cursor: pointer;
            width: 36px;
            height: 36px;
            border-radius: 50%;
            transition: all 0.15s;
            display: none;  /* Hidden by default, shown in fullscreen */
            align-items: center;
            justify-content: center;
        }
        .toolbar-toggle:hover { background: rgba(0,0,0,0.7); }
        .toolbar-toggle:active { transform: translateX(-50%) scale(0.9); }
        /* Zoom indicator */
        .zoom-indicator {
            position: fixed;
            bottom: 80px;
            left: 50%;
            transform: translateX(-50%);
            background: rgba(0,0,0,0.7);
            color: #fff;
            padding: 6px 14px;
            border-radius: 20px;
            font-size: 14px;
            z-index: 1002;
            display: none;
            pointer-events: none;
        }
        .zoom-indicator.visible { display: block; }
        .touch-indicator {
            position: fixed;
            width: 30px;
            height: 30px;
            border: 2px solid #e94560;
            border-radius: 50%;
            pointer-events: none;
            display: none;
            transform: translate(-50%, -50%);
            z-index: 999;
        }
        .cursor-overlay {
            position: fixed;
            width: 24px;
            height: 24px;
            pointer-events: none;
            display: none;
            z-index: 1002;
            background: url('data:image/svg+xml,<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><path fill="%23fff" stroke="%23000" stroke-width="1.5" d="M4 4l7 17 2.5-6.5L20 12z"/></svg>') no-repeat;
            background-size: contain;
            filter: drop-shadow(2px 2px 3px rgba(0,0,0,0.6));
            transform-origin: 0 0;
        }
        .cursor-overlay.active { display: block; }
        /* Mobile responsive */
        @media (max-width: 768px) {
            .page { padding: 10px; }
            .header { flex-direction: column; gap: 12px; padding: 12px; }
            .header h1 { font-size: 20px; }
            .search-box { width: 100%; }
            .search-input { font-size: 14px; }
            .device-grid { grid-template-columns: 1fr !important; gap: 10px; }
            .device-card { padding: 14px; }
            .device-card .name { font-size: 15px; }
            .device-card .ip { font-size: 12px; }
            .device-card .meta-row { flex-wrap: wrap; gap: 8px; }
            .device-card .active-window { font-size: 11px; }
            .screen-toolbar { padding: 8px 12px; }
            .back-btn { padding: 6px 12px; font-size: 13px; }
            .toolbar-info .device-name { font-size: 13px; }
            .toolbar-info .conn-info { font-size: 11px; }
            .screen-status { font-size: 11px; padding: 4px 10px; }
            .fullscreen-btn { font-size: 16px; padding: 4px 8px; }
            .pagination { flex-wrap: wrap; gap: 4px; }
            .pagination button { padding: 6px 10px; font-size: 12px; }
            .login-form { padding: 24px; margin: 10px; }
            .login-form h1 { font-size: 22px; }
        }
        @media (max-width: 480px) {
            .header h1 { font-size: 18px; }
            .device-card .info-row { flex-direction: column; gap: 4px; }
            .toolbar-right { gap: 8px; }
            .mobile-btn { width: 44px; height: 44px; font-size: 18px; }
            /* Small screens: hide toolbar buttons, show floating toggle */
            .toolbar-btn-bar { display: none; }
            .toolbar-toggle { display: flex; }
        }
    </style>
</head>
<body>
)HTML";

    // Part 6: HTML body
    html += R"HTML(
    <div id="login-page" class="page active">
        <div class="login-form">
            <h1>SimpleRemoter</h1>
            <div id="ws-status" class="conn-status disconnected">Connecting...</div>
            <input type="text" id="username" placeholder="Username" value="admin">
            <input type="password" id="password" placeholder="Password">
            <button id="login-btn" onclick="login()" disabled>Login</button>
            <div id="login-error" class="error-msg"></div>
        </div>
    </div>
    <div id="devices-page" class="page">
        <div class="page-header">
            <h2>Devices <span id="device-count" class="device-count">0</span></h2>
            <div class="toolbar">
                <div class="search-box">
                    <input type="text" id="search-input" placeholder="Search by name, IP, OS..." oninput="onSearchChange()">
                    <button class="search-clear" id="search-clear" onclick="clearSearch()">&times;</button>
                </div>
                <div class="view-toggle">
                    <button id="view-grid" class="view-btn active" onclick="setViewMode('grid')" title="Grid View">Grid</button>
                    <button id="view-list" class="view-btn" onclick="setViewMode('list')" title="List View">List</button>
                </div>
                <button class="refresh-btn" onclick="getDevices()">Refresh</button>
                <button class="logout-btn" onclick="logout()">Logout</button>
            </div>
        </div>
        <div id="stats-bar" class="stats-bar">
            <div class="stat">Total: <strong id="stat-total">0</strong></div>
            <div class="stat">Showing: <strong id="stat-showing">0</strong></div>
        </div>
        <div id="device-list" class="device-grid"></div>
        <div id="pagination" class="pagination"></div>
    </div>
    <div id="screen-page" class="page">
        <div class="screen-toolbar">
            <button class="back-btn" onclick="disconnect()">Back</button>
            <div class="toolbar-info">
                <div id="device-name" class="device-name"></div>
                <div id="frame-info" class="conn-info"></div>
            </div>
            <div class="toolbar-right">
                <span id="screen-status" class="screen-status connecting">Connecting...</span>
                <button class="toolbar-btn-bar" id="btn-mouse-bar" onclick="toggleControl()" title="Mouse Control">&#x1F5B1;</button>
                <button class="toolbar-btn-bar" id="btn-keyboard-bar" onclick="toggleKeyboard()" title="Keyboard" disabled>&#x2328;</button>
                <button class="fullscreen-btn" onclick="toggleFullscreen()" title="Fullscreen (F11)">&#x26F6;</button>
            </div>
        </div>
        <div class="canvas-container"><canvas id="screen-canvas"></canvas><div class="cursor-overlay" id="cursor-overlay"></div></div>
        <div class="touch-indicator" id="touch-indicator"></div>
        <button class="toolbar-toggle" id="toolbar-toggle" onclick="toggleFloatingToolbar()">&#x2022;&#x2022;&#x2022;</button>
        <div class="floating-toolbar" id="floating-toolbar">
            <button class="toolbar-btn" onclick="sendRdpReset()" title="RDP Reset">&#x21BB;</button>
            <button class="toolbar-btn" id="btn-mouse" onclick="toggleControl()" title="Mouse Control">&#x1F5B1;</button>
            <button class="toolbar-btn" id="btn-keyboard" onclick="toggleKeyboard()" title="Keyboard" disabled>&#x2328;</button>
            <button class="toolbar-btn" onclick="disconnect()" title="Disconnect">&#x2715;</button>
        </div>
        <div class="zoom-indicator" id="zoom-indicator">100%</div>
        <input type="text" id="mobile-keyboard" style="position:fixed;left:-9999px;opacity:0;" autocomplete="off" autocorrect="off" autocapitalize="off">
    </div>
)HTML";

    // Part 7: JavaScript - State and WebSocket
    html += R"HTML(
    <script>
        let ws = null, token = null, decoder = null, devices = [], currentDevice = null;
        let frameCount = 0, lastFrameTime = 0, fps = 0, pingInterval = null;
        const canvas = document.getElementById('screen-canvas');
        const ctx = canvas.getContext('2d');

        // Pagination and filter state
        let currentPage = 1;
        let viewMode = 'grid';  // 'grid' or 'list'
        let searchQuery = '';
        let challengeNonce = '';  // Challenge nonce from server
        let passwordHash = '';    // SHA256 of password (computed once)

        // SHA256 using Web Crypto API
        // SHA256 implementation (works in non-secure contexts)
        async function sha256(message) {
            // Try Web Crypto API first (secure contexts only)
            if (window.crypto && window.crypto.subtle) {
                try {
                    const msgBuffer = new TextEncoder().encode(message);
                    const hashBuffer = await crypto.subtle.digest('SHA-256', msgBuffer);
                    const hashArray = Array.from(new Uint8Array(hashBuffer));
                    return hashArray.map(b => b.toString(16).padStart(2, '0')).join('');
                } catch (e) { /* fallback below */ }
            }
            // Fallback: pure JS SHA256
            return sha256_js(message);
        }

        // Pure JS SHA256 implementation
        function sha256_js(str) {
            const K = [
                0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
                0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
                0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
                0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
                0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
                0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
                0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
                0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
            ];
            const utf8 = unescape(encodeURIComponent(str));
            const bytes = [];
            for (let i = 0; i < utf8.length; i++) bytes.push(utf8.charCodeAt(i));
            bytes.push(0x80);
            while ((bytes.length + 8) % 64 !== 0) bytes.push(0);
            const bitLen = (utf8.length * 8);
            for (let i = 7; i >= 0; i--) bytes.push((bitLen / Math.pow(2, i * 8)) & 0xff);
            let [h0, h1, h2, h3, h4, h5, h6, h7] = [0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19];
            const rotr = (x, n) => ((x >>> n) | (x << (32 - n))) >>> 0;
            for (let i = 0; i < bytes.length; i += 64) {
                const w = [];
                for (let j = 0; j < 16; j++) w[j] = (bytes[i+j*4]<<24)|(bytes[i+j*4+1]<<16)|(bytes[i+j*4+2]<<8)|bytes[i+j*4+3];
                for (let j = 16; j < 64; j++) {
                    const s0 = rotr(w[j-15],7) ^ rotr(w[j-15],18) ^ (w[j-15]>>>3);
                    const s1 = rotr(w[j-2],17) ^ rotr(w[j-2],19) ^ (w[j-2]>>>10);
                    w[j] = (w[j-16] + s0 + w[j-7] + s1) >>> 0;
                }
                let [a,b,c,d,e,f,g,h] = [h0,h1,h2,h3,h4,h5,h6,h7];
                for (let j = 0; j < 64; j++) {
                    const S1 = rotr(e,6) ^ rotr(e,11) ^ rotr(e,25);
                    const ch = (e & f) ^ (~e & g);
                    const t1 = (h + S1 + ch + K[j] + w[j]) >>> 0;
                    const S0 = rotr(a,2) ^ rotr(a,13) ^ rotr(a,22);
                    const maj = (a & b) ^ (a & c) ^ (b & c);
                    const t2 = (S0 + maj) >>> 0;
                    [h,g,f,e,d,c,b,a] = [g,f,e,(d+t1)>>>0,c,b,a,(t1+t2)>>>0];
                }
                [h0,h1,h2,h3,h4,h5,h6,h7] = [(h0+a)>>>0,(h1+b)>>>0,(h2+c)>>>0,(h3+d)>>>0,(h4+e)>>>0,(h5+f)>>>0,(h6+g)>>>0,(h7+h)>>>0];
            }
            return [h0,h1,h2,h3,h4,h5,h6,h7].map(x => x.toString(16).padStart(8,'0')).join('');
        }

        function getPageSize() {
            return viewMode === 'grid' ? 12 : 50;
        }

        function connectWebSocket() {
            const protocol = location.protocol === 'https:' ? 'wss:' : 'ws:';
            updateWsStatus('connecting');
            try {
                ws = new WebSocket(protocol + '//' + location.host + '/ws');
            } catch (e) {
                updateWsStatus('disconnected');
                setTimeout(connectWebSocket, 3000);
                return;
            }
            ws.binaryType = 'arraybuffer';
            ws.onopen = () => {
                updateWsStatus('connected');
                startPingInterval();
                // Auto-restore session if token exists
                if (token) {
                    showPage('devices-page');
                    getDevices();
                }
            };
            ws.onclose = () => { stopPingInterval(); updateWsStatus('disconnected'); setTimeout(connectWebSocket, 3000); };
            ws.onerror = (e) => console.error('WS error:', e);
            ws.onmessage = (event) => {
                if (typeof event.data === 'string') handleSignaling(JSON.parse(event.data));
                else handleBinaryFrame(event.data);
            };
        }

        function updateWsStatus(status) {
            const el = document.getElementById('ws-status');
            const btn = document.getElementById('login-btn');
            if (status === 'connected') {
                el.textContent = 'Connected'; el.className = 'conn-status connected'; btn.disabled = false;
            } else if (status === 'connecting') {
                el.textContent = 'Connecting...'; el.className = 'conn-status'; btn.disabled = true;
            } else {
                el.textContent = 'Disconnected'; el.className = 'conn-status disconnected'; btn.disabled = true;
            }
        }
)HTML";

    // Part 8: JavaScript - Signaling handler
    html += R"HTML(
        function handleSignaling(msg) {
            switch (msg.cmd) {
                case 'challenge':
                    challengeNonce = msg.nonce || '';
                    console.log('Received challenge nonce');
                    break;
                case 'login_result':
                    if (msg.ok) {
                        token = msg.token;
                        sessionStorage.setItem('token', token);
                        document.getElementById('login-error').textContent = '';
                        showPage('devices-page');
                        getDevices();
                    } else {
                        document.getElementById('login-error').textContent = msg.msg || 'Login failed';
                    }
                    break;
                case 'device_list':
                    if (msg.ok === false) {
                        // Token invalid (e.g., server restarted), go back to login
                        token = null;
                        sessionStorage.removeItem('token');
                        showPage('login-page');
                        break;
                    }
                    devices = msg.devices || [];
                    // Keep current page (renderDeviceList adjusts if out of range)
                    renderDeviceList();
                    break;
                case 'connect_result':
                    if (!token) break;
                    if (msg.ok) {
                        // Resolution may not be available yet (first connection)
                        if (msg.width && msg.height) {
                            updateScreenStatus('connected');
                            initDecoder(msg.width, msg.height);
                        } else {
                            // Wait for resolution_changed message
                            updateScreenStatus('waiting', 'Waiting for video...');
                        }
                    } else {
                        updateScreenStatus('error', msg.msg);
                        setTimeout(() => showPage('devices-page'), 2000);
                    }
                    break;
                case 'disconnect_result':
                    // Only navigate if authenticated
                    if (!token) break;
                    showPage('devices-page');
                    getDevices();
                    break;
                case 'resolution_changed':
                    updateScreenStatus('connected');
                    initDecoder(msg.width, msg.height);
                    break;
                case 'device_offline':
                    if (!token) break;
                    updateScreenStatus('error', 'Device offline');
                    setTimeout(() => { showPage('devices-page'); getDevices(); }, 2000);
                    break;
                case 'device_update':
                    // Only update if on devices page
                    if (document.getElementById('devices-page').classList.contains('active')) {
                        updateDeviceInfo(msg.id, msg.rtt, msg.activeWindow);
                    }
                    break;
                case 'devices_changed':
                    // Refresh device list when devices come online/offline
                    if (document.getElementById('devices-page').classList.contains('active')) {
                        getDevices();
                    }
                    break;
            }
        }
)HTML";

    // Part 9: JavaScript - H264 decoder
    html += R"HTML(
        function checkWebCodecs() {
            if (!('VideoDecoder' in window)) return { supported: false, reason: 'VideoDecoder not available' };
            return { supported: true };
        }

        function initDecoder(width, height) {
            // Clear canvas before resizing to prevent residual content
            ctx.setTransform(1, 0, 0, 1, 0, 0);  // Reset transform
            ctx.clearRect(0, 0, canvas.width, canvas.height);

            canvas.width = width;
            canvas.height = height;

            // Clear again after resize (in case of smaller resolution)
            ctx.fillStyle = '#000';
            ctx.fillRect(0, 0, width, height);

            // Set up vertical flip transform once (BMP is bottom-up)
            ctx.setTransform(1, 0, 0, -1, 0, height);
            if (decoder) { try { decoder.close(); } catch(e) {} }
            frameCount = 0;
            lastFrameTime = performance.now();
            decoder = new VideoDecoder({
                output: (frame) => {
                    ctx.drawImage(frame, 0, 0);
                    frame.close();
                    frameCount++;
                    const now = performance.now();
                    if (now - lastFrameTime >= 1000) {
                        fps = Math.round(frameCount * 1000 / (now - lastFrameTime));
                        frameCount = 0;
                        lastFrameTime = now;
                        document.getElementById('frame-info').textContent = width + 'x' + height + ' @ ' + fps + ' fps';
                    }
                },
                error: (e) => { console.error('Decoder error:', e); updateScreenStatus('error', 'Decode error'); }
            });
            decoder.configure({
                codec: 'avc1.42E01E',
                codedWidth: width,
                codedHeight: height,
                optimizeForLatency: true
            });
        }

        function handleBinaryFrame(data) {
            if (!decoder || decoder.state !== 'configured') return;
            const view = new DataView(data);
            const deviceId = view.getUint32(0, true);
            const frameType = view.getUint8(4);
            const dataLen = view.getUint32(5, true);
            const h264Data = new Uint8Array(data, 9, dataLen);
            try {
                decoder.decode(new EncodedVideoChunk({
                    type: frameType === 1 ? 'key' : 'delta',
                    timestamp: performance.now() * 1000,
                    data: h264Data
                }));
            } catch (e) { console.error('Decode error:', e); }
        }
)HTML";

    // Part 10: JavaScript - Filter and pagination
    html += R"HTML(
        function getFilteredDevices() {
            let filtered = devices;
            if (searchQuery) {
                const q = searchQuery.toLowerCase();
                filtered = filtered.filter(d =>
                    (d.name && d.name.toLowerCase().includes(q)) ||
                    (d.ip && d.ip.toLowerCase().includes(q)) ||
                    (d.os && d.os.toLowerCase().includes(q)) ||
                    (d.location && d.location.toLowerCase().includes(q)) ||
                    (d.version && d.version.toLowerCase().includes(q)) ||
                    (d.activeWindow && d.activeWindow.toLowerCase().includes(q))
                );
            }
            return filtered;
        }

        function onSearchChange() {
            searchQuery = document.getElementById('search-input').value;
            document.getElementById('search-clear').style.display = searchQuery ? 'block' : 'none';
            currentPage = 1;
            renderDeviceList();
        }

        function clearSearch() {
            document.getElementById('search-input').value = '';
            onSearchChange();
        }

        function setViewMode(mode) {
            viewMode = mode;
            document.getElementById('view-grid').classList.toggle('active', mode === 'grid');
            document.getElementById('view-list').classList.toggle('active', mode === 'list');
            const list = document.getElementById('device-list');
            list.classList.toggle('list-view', mode === 'list');
            currentPage = 1;
            renderDeviceList();
        }

        function goToPage(page) {
            currentPage = page;
            renderDeviceList();
            window.scrollTo({ top: 0, behavior: 'smooth' });
        }
)HTML";

    // Part 11: JavaScript - Render device list
    html += R"HTML(
        function getRttClass(rtt) {
            if (!rtt || rtt === '-') return '';
            const ms = parseInt(rtt);
            if (isNaN(ms)) return '';
            if (ms < 100) return 'rtt-good';      // Green: < 100ms
            if (ms < 300) return 'rtt-medium';    // Orange: 100-300ms
            return 'rtt-poor';                     // Red: > 300ms
        }

        function updateDeviceInfo(deviceId, rtt, activeWindow) {
            // Update device in array
            const device = devices.find(d => d.id === deviceId || d.id === String(deviceId));
            if (!device) return;

            let changed = false;
            if (rtt && device.rtt !== rtt) {
                device.rtt = rtt;
                changed = true;
            }
            if (activeWindow !== undefined && device.activeWindow !== activeWindow) {
                device.activeWindow = activeWindow;
                changed = true;
            }

            if (!changed) return;

            // Update DOM directly for efficiency (avoid full re-render)
            const cards = document.querySelectorAll('.device-card');
            for (const card of cards) {
                if (card.onclick && card.onclick.toString().includes(deviceId)) {
                    // Update RTT
                    const rttEl = card.querySelector('.meta-item.rtt');
                    if (rttEl && rtt) {
                        rttEl.textContent = 'RTT: ' + rtt;
                        rttEl.className = 'meta-item rtt ' + getRttClass(rtt);
                    }
                    // Update active window
                    let winEl = card.querySelector('.active-window');
                    if (activeWindow) {
                        if (!winEl) {
                            winEl = document.createElement('div');
                            winEl.className = 'active-window';
                            card.appendChild(winEl);
                        }
                        winEl.textContent = activeWindow;
                        winEl.title = activeWindow;
                    } else if (winEl) {
                        winEl.remove();
                    }
                    break;
                }
            }
        }

        function renderDeviceList() {
            const filtered = getFilteredDevices();
            const ps = getPageSize();
            const totalPages = Math.ceil(filtered.length / ps) || 1;
            if (currentPage > totalPages) currentPage = totalPages;

            const start = (currentPage - 1) * ps;
            const pageDevices = filtered.slice(start, start + ps);

            // Update stats
            document.getElementById('device-count').textContent = devices.length;
            document.getElementById('stat-total').textContent = devices.length;
            document.getElementById('stat-showing').textContent = filtered.length;

            // Render devices
            const list = document.getElementById('device-list');
            if (pageDevices.length === 0) {
                list.innerHTML = '<div class="no-devices"><h3>No devices found</h3><p>Try adjusting your search or filter</p></div>';
            } else {
                list.innerHTML = pageDevices.map(d => {
                    const screenInfo = d.screen || '-';  // e.g. "2:3840x1080"
                    const loc = d.location || '-';
                    const rtt = d.rtt || '-';
                    const ver = d.version || '-';
                    const activeWin = d.activeWindow || '';
                    return '<div class="device-card" onclick="connectDevice(\'' + d.id + '\')">' +
                        '<h3>' + escapeHtml(d.name || 'Unknown') + '</h3>' +
                        '<div class="info-row">' +
                            '<div class="info"><span class="info-label">IP:</span> ' + escapeHtml(d.ip || '-') + '</div>' +
                            '<div class="info"><span class="info-label">Loc:</span> ' + escapeHtml(loc) + '</div>' +
                        '</div>' +
                        '<div class="info"><span class="info-label">OS:</span> ' + escapeHtml(d.os || '-') + '</div>' +
                        '<div class="meta-row">' +
                            '<span class="meta-item rtt ' + getRttClass(rtt) + '">RTT: ' + escapeHtml(rtt) + '</span>' +
                            '<span class="meta-item">Ver: ' + escapeHtml(ver) + '</span>' +
                            '<span class="meta-item">' + screenInfo + '</span>' +
                        '</div>' +
                        (activeWin ? '<div class="active-window" title="' + escapeHtml(activeWin) + '">' + escapeHtml(activeWin) + '</div>' : '') +
                    '</div>';
                }).join('');
            }

            // Render pagination
            renderPagination(totalPages);
        }
)HTML";

    // Part 12: JavaScript - Pagination render
    html += R"HTML(
        function renderPagination(totalPages) {
            const pagination = document.getElementById('pagination');
            if (totalPages <= 1) {
                pagination.innerHTML = '';
                return;
            }

            let html = '';
            html += '<button onclick="goToPage(1)" ' + (currentPage === 1 ? 'disabled' : '') + '>First</button>';
            html += '<button onclick="goToPage(' + (currentPage - 1) + ')" ' + (currentPage === 1 ? 'disabled' : '') + '>Prev</button>';

            // Page numbers
            let startPage = Math.max(1, currentPage - 2);
            let endPage = Math.min(totalPages, startPage + 4);
            if (endPage - startPage < 4) startPage = Math.max(1, endPage - 4);

            for (let i = startPage; i <= endPage; i++) {
                html += '<button onclick="goToPage(' + i + ')" class="' + (i === currentPage ? 'active' : '') + '">' + i + '</button>';
            }

            html += '<button onclick="goToPage(' + (currentPage + 1) + ')" ' + (currentPage === totalPages ? 'disabled' : '') + '>Next</button>';
            html += '<button onclick="goToPage(' + totalPages + ')" ' + (currentPage === totalPages ? 'disabled' : '') + '>Last</button>';
            html += '<span class="page-info">Page ' + currentPage + ' of ' + totalPages + '</span>';

            pagination.innerHTML = html;
        }
)HTML";

    // Part 13: JavaScript - UI functions
    html += R"HTML(
        function showPage(pageId) {
            document.querySelectorAll('.page').forEach(p => p.classList.remove('active'));
            document.getElementById(pageId).classList.add('active');
        }

        function updateScreenStatus(status, msg) {
            const el = document.getElementById('screen-status');
            el.className = 'screen-status ' + status;
            if (status === 'connected') el.textContent = 'Connected';
            else if (status === 'connecting') el.textContent = 'Connecting...';
            else if (status === 'waiting') el.textContent = msg || 'Waiting...';
            else el.textContent = msg || 'Error';
        }

        async function login() {
            const username = document.getElementById('username').value;
            const password = document.getElementById('password').value;
            if (!username || !password) { document.getElementById('login-error').textContent = 'Please enter credentials'; return; }
            if (!ws || ws.readyState !== WebSocket.OPEN) { document.getElementById('login-error').textContent = 'Not connected'; return; }
            if (!challengeNonce) { document.getElementById('login-error').textContent = 'No challenge received'; return; }

            // Compute password hash (same as server stores)
            passwordHash = await sha256(password);
            // Compute response: SHA256(passwordHash + nonce)
            const response = await sha256(passwordHash + challengeNonce);
            ws.send(JSON.stringify({ cmd: 'login', username, response, nonce: challengeNonce }));
        }

        function logout() {
            // Close and reconnect WebSocket to get new challenge
            if (ws) {
                ws.onclose = null;  // Prevent auto-reconnect delay
                ws.close();
            }
            challengeNonce = '';
            connectWebSocket();  // Reconnect immediately
            token = null;
            sessionStorage.removeItem('token');
            devices = [];
            showPage('login-page');
        }

        function getDevices() {
            if (!ws || ws.readyState !== WebSocket.OPEN || !token) return;
            ws.send(JSON.stringify({ cmd: 'get_devices', token }));
        }

        function connectDevice(id) {
            const compat = checkWebCodecs();
            if (!compat.supported) { alert('Browser does not support H264: ' + compat.reason); return; }
            currentDevice = devices.find(d => d.id === id || d.id === String(id));
            if (!currentDevice || !currentDevice.online) {
                alert('Device is offline');
                return;
            }
            document.getElementById('device-name').textContent = currentDevice.name;
            document.getElementById('frame-info').textContent = '';
            updateScreenStatus('connecting');
            showPage('screen-page');
            ws.send(JSON.stringify({ cmd: 'connect', id: String(id), token }));
        }

        function toggleFullscreen() {
            const el = document.getElementById('screen-page');
            const isFullscreen = document.fullscreenElement || document.webkitFullscreenElement;
            const isPseudo = el.classList.contains('pseudo-fullscreen');

            // Try native fullscreen first
            if (!isPseudo && (el.requestFullscreen || el.webkitRequestFullscreen)) {
                if (!isFullscreen) {
                    (el.requestFullscreen || el.webkitRequestFullscreen).call(el).catch(() => {
                        // Native fullscreen failed (iOS), use pseudo-fullscreen
                        el.classList.add('pseudo-fullscreen');
                        window.scrollTo(0, 1);
                    });
                } else {
                    if (document.exitFullscreen) document.exitFullscreen();
                    else if (document.webkitExitFullscreen) document.webkitExitFullscreen();
                }
            } else {
                // Toggle pseudo-fullscreen (iOS fallback)
                el.classList.toggle('pseudo-fullscreen');
                if (el.classList.contains('pseudo-fullscreen')) window.scrollTo(0, 1);
            }
        }
)HTML";

    // Part 14: JavaScript - Toolbar, zoom and touch
    html += R"HTML(
        // Double-tap to exit pseudo-fullscreen on mobile (only when control mode is OFF)
        let lastTapTime = 0;
        document.getElementById('screen-canvas').addEventListener('touchend', function(e) {
            // When control mode is enabled, double-tap sends double-click, not exit fullscreen
            if (controlEnabled) return;
            const el = document.getElementById('screen-page');
            if (!el.classList.contains('pseudo-fullscreen')) return;
            // Only exit fullscreen on single-finger double tap, not during pinch
            if (touchState.touchCount > 1 || zoomState.isPinching || zoomState.scale > 1) return;
            const now = Date.now();
            if (now - lastTapTime < 300 && e.changedTouches.length === 1) {
                el.classList.remove('pseudo-fullscreen');
                e.preventDefault();
            }
            lastTapTime = now;
        });

        document.addEventListener('keydown', function(e) {
            if (e.key === 'F11' && document.getElementById('screen-page').classList.contains('active')) {
                e.preventDefault();
                toggleFullscreen();
            }
        });

        // Control mode state (mouse/keyboard control)
        let controlEnabled = false;

        // Floating toolbar state
        let toolbarVisible = false;
        let toolbarHideTimer = null;

        function toggleFloatingToolbar() {
            const toolbar = document.getElementById('floating-toolbar');
            toolbarVisible = !toolbarVisible;
            toolbar.classList.toggle('visible', toolbarVisible);

            // Auto-hide after 4 seconds
            if (toolbarHideTimer) clearTimeout(toolbarHideTimer);
            if (toolbarVisible) {
                toolbarHideTimer = setTimeout(() => {
                    toolbarVisible = false;
                    toolbar.classList.remove('visible');
                }, 4000);
            }
        }

        function sendRdpReset() {
            if (ws && ws.readyState === WebSocket.OPEN && token) {
                ws.send(JSON.stringify({ cmd: 'rdp_reset', token }));
                // Visual feedback - rotate icon
                const btn = event.target;
                btn.style.transform = 'rotate(360deg)';
                btn.style.transition = 'transform 0.5s';
                setTimeout(() => {
                    btn.style.transform = '';
                    btn.style.transition = '';
                }, 500);
            }
        }

        // Detect touch device (mobile/tablet)
        const isTouchDevice = ('ontouchstart' in window) || (navigator.maxTouchPoints > 0);

        function toggleControl() {
            controlEnabled = !controlEnabled;
            // Update floating toolbar buttons
            const btnMouse = document.getElementById('btn-mouse');
            const btnKeyboard = document.getElementById('btn-keyboard');
            btnMouse.classList.toggle('active', controlEnabled);
            btnKeyboard.disabled = !controlEnabled;
            // Update top toolbar buttons (sync state)
            const btnMouseBar = document.getElementById('btn-mouse-bar');
            const btnKeyboardBar = document.getElementById('btn-keyboard-bar');
            btnMouseBar.classList.toggle('active', controlEnabled);
            btnKeyboardBar.disabled = !controlEnabled;
            // Cursor handling
            const canvas = document.getElementById('screen-canvas');
            const cursorOverlay = document.getElementById('cursor-overlay');
            // Touch devices: hide browser cursor, show overlay (touchpad mode)
            // Desktop: keep browser cursor visible, no overlay needed (remote shows cursor)
            canvas.style.cursor = (controlEnabled && isTouchDevice) ? 'none' : 'default';
            cursorOverlay.classList.toggle('active', controlEnabled && isTouchDevice);
        }

        // Update cursor overlay position (accounting for zoom/pan transform)
        // Only used on touch devices (touchpad mode)
        function updateCursorOverlay(canvasX, canvasY) {
            if (!controlEnabled || !isTouchDevice) return;
            const canvas = document.getElementById('screen-canvas');
            const cursorOverlay = document.getElementById('cursor-overlay');

            // Get canvas base rect (without transform, use container)
            const container = document.querySelector('.canvas-container');
            const containerRect = container.getBoundingClientRect();

            // Calculate canvas position within container (centered)
            const canvasDisplayWidth = canvas.offsetWidth;
            const canvasDisplayHeight = canvas.offsetHeight;
            const canvasLeft = containerRect.left + (containerRect.width - canvasDisplayWidth) / 2;
            const canvasTop = containerRect.top + (containerRect.height - canvasDisplayHeight) / 2;

            // Convert canvas coords to position on unzoomed canvas
            const relX = canvasX / canvas.width;  // 0-1
            const relY = canvasY / canvas.height; // 0-1

            // Position on unzoomed canvas (in pixels from canvas top-left)
            const unzoomedX = relX * canvasDisplayWidth;
            const unzoomedY = relY * canvasDisplayHeight;

            // Apply zoom transform
            // Transform origin
            const originX = canvasDisplayWidth * zoomState.originX / 100;
            const originY = canvasDisplayHeight * zoomState.originY / 100;

            // Scale around origin, then translate
            const scaledX = originX + (unzoomedX - originX) * zoomState.scale + zoomState.translateX * zoomState.scale;
            const scaledY = originY + (unzoomedY - originY) * zoomState.scale + zoomState.translateY * zoomState.scale;

            // Final screen position
            const screenX = canvasLeft + scaledX;
            const screenY = canvasTop + scaledY;

            cursorOverlay.style.left = screenX + 'px';
            cursorOverlay.style.top = screenY + 'px';
        }

        // Pinch-to-zoom state
        let zoomState = {
            scale: 1,
            minScale: 1,
            maxScale: 4,
            translateX: 0,
            translateY: 0,
            lastPinchDist: 0,
            isPinching: false,
            pinchCenterX: 0,
            pinchCenterY: 0,
            // Transform origin relative to canvas (percentage)
            originX: 50,
            originY: 50
        };
        const zoomIndicator = document.getElementById('zoom-indicator');
        let zoomIndicatorTimer = null;

        function showZoomIndicator() {
            zoomIndicator.textContent = Math.round(zoomState.scale * 100) + '%';
            zoomIndicator.classList.add('visible');
            if (zoomIndicatorTimer) clearTimeout(zoomIndicatorTimer);
            zoomIndicatorTimer = setTimeout(() => {
                zoomIndicator.classList.remove('visible');
            }, 1500);
        }

        function resetZoom() {
            zoomState.scale = 1;
            zoomState.translateX = 0;
            zoomState.translateY = 0;
            applyZoomTransform();
            showZoomIndicator();
        }

        // Track previous scale to detect zoom reset
        let prevScale = 1;

        function applyZoomTransform() {
            const container = document.querySelector('.canvas-container');
            if (zoomState.scale === 1) {
                canvas.style.transform = '';
                canvas.style.transformOrigin = '';
                // Reset all zoom state when scale returns to 1
                zoomState.originX = 50;
                zoomState.originY = 50;
                zoomState.translateX = 0;
                zoomState.translateY = 0;
                // If we just returned from zoomed state, force cursor overlay update
                if (prevScale !== 1 && controlEnabled) {
                    console.log('[Zoom] Reset from ' + prevScale.toFixed(2) + ' to 1, updating cursor overlay');
                    updateCursorOverlay(cursorState.x, cursorState.y);
                }
                prevScale = 1;
            } else {
                prevScale = zoomState.scale;
                // Clamp translate to prevent canvas from going out of view
                const rect = container.getBoundingClientRect();
                const maxTranslateX = (canvas.width * zoomState.scale - rect.width) / 2 / zoomState.scale;
                const maxTranslateY = (canvas.height * zoomState.scale - rect.height) / 2 / zoomState.scale;

                if (canvas.width * zoomState.scale > rect.width) {
                    zoomState.translateX = Math.max(-maxTranslateX, Math.min(maxTranslateX, zoomState.translateX));
                } else {
                    zoomState.translateX = 0;
                }
                if (canvas.height * zoomState.scale > rect.height) {
                    zoomState.translateY = Math.max(-maxTranslateY, Math.min(maxTranslateY, zoomState.translateY));
                } else {
                    zoomState.translateY = 0;
                }

                // Use pinch center as transform origin
                canvas.style.transformOrigin = zoomState.originX + '% ' + zoomState.originY + '%';
                canvas.style.transform = 'scale(' + zoomState.scale + ') translate(' + zoomState.translateX + 'px, ' + zoomState.translateY + 'px)';
            }
        }

        function getPinchDistance(touches) {
            const dx = touches[0].clientX - touches[1].clientX;
            const dy = touches[0].clientY - touches[1].clientY;
            return Math.sqrt(dx * dx + dy * dy);
        }

        function getPinchCenter(touches) {
            return {
                x: (touches[0].clientX + touches[1].clientX) / 2,
                y: (touches[0].clientY + touches[1].clientY) / 2
            };
        }

        // Mobile touch handling (touchpad mode - like Microsoft Remote Desktop)
        // Cursor position is separate from finger position
        let cursorState = { x: 0, y: 0, initialized: false };  // Remote cursor position
        let touchState = {
            lastTap: 0, longPressTimer: null, isDragging: false,
            lastX: 0, lastY: 0,  // Last touch screen position (for delta calculation)
            touchCount: 0,
            startX: 0, startY: 0,  // Touch start position (to detect tap vs drag)
            moved: false,  // Did finger move significantly?
            dragHoldTimer: null  // Timer for double-tap-hold drag detection
        };
        const touchIndicator = document.getElementById('touch-indicator');
        const mobileKeyboard = document.getElementById('mobile-keyboard');

        // Initialize cursor to center of screen
        function initCursor() {
            if (!cursorState.initialized && canvas.width > 0) {
                cursorState.x = Math.round(canvas.width / 2);
                cursorState.y = Math.round(canvas.height / 2);
                cursorState.initialized = true;
                updateCursorOverlay(cursorState.x, cursorState.y);
            }
        }

        // Move cursor by delta (touchpad mode)
        function moveCursorBy(dx, dy) {
            initCursor();
            // Sensitivity multiplier (adjust for comfortable control)
            const sensitivity = 1.5;
            cursorState.x = Math.max(0, Math.min(canvas.width, cursorState.x + dx * sensitivity));
            cursorState.y = Math.max(0, Math.min(canvas.height, cursorState.y + dy * sensitivity));
            updateCursorOverlay(cursorState.x, cursorState.y);
            // Send move to remote
            sendMouse('move', Math.round(cursorState.x), Math.round(cursorState.y), 0);
        }

        // Click at current cursor position
        function clickAtCursor(button) {
            initCursor();
            const x = Math.round(cursorState.x);
            const y = Math.round(cursorState.y);
            sendMouse('down', x, y, button);
            sendMouse('up', x, y, button);
        }

        function dblClickAtCursor() {
            initCursor();
            const x = Math.round(cursorState.x);
            const y = Math.round(cursorState.y);
            sendMouse('dblclick', x, y, 0);
        }

        function getTouchPos(touch) {
            const rect = canvas.getBoundingClientRect();
            const scaleX = canvas.width / rect.width;
            const scaleY = canvas.height / rect.height;
            return {
                x: Math.round((touch.clientX - rect.left) * scaleX),
                y: Math.round((touch.clientY - rect.top) * scaleY)
            };
        }

        function sendMouse(type, x, y, button, delta) {
            if (!controlEnabled) return;  // Control mode required
            // Update cursor overlay position
            updateCursorOverlay(x, y);
            if (ws && ws.readyState === WebSocket.OPEN && token) {
                const msg = { cmd: 'mouse', token, type, x, y, button: button || 0 };
                if (delta !== undefined) msg.delta = delta;
                ws.send(JSON.stringify(msg));
            }
        }

        function sendKey(keyCode, isDown, altKey) {
            if (!controlEnabled) return;  // Control mode required
            // Filter Windows keys (handled locally, not sent to remote)
            if (keyCode === 91 || keyCode === 92) return;  // VK_LWIN, VK_RWIN
            if (ws && ws.readyState === WebSocket.OPEN && token) {
                ws.send(JSON.stringify({ cmd: 'key', token, keyCode, down: isDown, alt: !!altKey }));
            }
        }

        function showTouchIndicator(x, y) {
            touchIndicator.style.left = x + 'px';
            touchIndicator.style.top = y + 'px';
            touchIndicator.style.display = 'block';
            setTimeout(() => touchIndicator.style.display = 'none', 200);
        }
)HTML";

    // Part 15: JavaScript - Touch and mouse event handlers
    html += R"HTML(
        canvas.addEventListener('touchstart', function(e) {
            e.preventDefault();
            e.stopPropagation();  // Prevent exiting fullscreen
            touchState.touchCount = e.touches.length;

            if (e.touches.length === 2) {
                // Two finger touch - start pinch zoom
                zoomState.isPinching = true;
                zoomState.lastPinchDist = getPinchDistance(e.touches);
                const center = getPinchCenter(e.touches);
                zoomState.pinchCenterX = center.x;
                zoomState.pinchCenterY = center.y;

                // Calculate pinch center relative to canvas for transform-origin
                // Only set origin when starting a new zoom (scale == 1)
                if (zoomState.scale === 1) {
                    const rect = canvas.getBoundingClientRect();
                    const relX = (center.x - rect.left) / rect.width * 100;
                    const relY = (center.y - rect.top) / rect.height * 100;
                    // Clamp to canvas bounds
                    zoomState.originX = Math.max(0, Math.min(100, relX));
                    zoomState.originY = Math.max(0, Math.min(100, relY));
                }

                // Cancel any pending single-touch actions
                if (touchState.longPressTimer) {
                    clearTimeout(touchState.longPressTimer);
                    touchState.longPressTimer = null;
                }
                if (touchState.dragHoldTimer) {
                    clearTimeout(touchState.dragHoldTimer);
                    touchState.dragHoldTimer = null;
                }
                return;
            }

            // Single finger touch - touchpad mode
            initCursor();
            const touch = e.touches[0];
            // Store screen coordinates for delta calculation
            touchState.startX = touch.clientX;
            touchState.startY = touch.clientY;
            touchState.lastX = touch.clientX;
            touchState.lastY = touch.clientY;
            touchState.isDragging = false;
            touchState.moved = false;
            // Initialize pan center for zoom pan detection
            zoomState.pinchCenterX = touch.clientX;
            zoomState.pinchCenterY = touch.clientY;

            // Clear any pending timers
            if (touchState.dragHoldTimer) {
                clearTimeout(touchState.dragHoldTimer);
                touchState.dragHoldTimer = null;
            }

            const now = Date.now();
            const timeSinceLastTap = now - touchState.lastTap;
            const isDoubleTap = (timeSinceLastTap < 400);

            if (isDoubleTap && controlEnabled) {
                // Double-tap detected - wait to see if user holds (drag) or releases quickly (double-click)
                // Microsoft RD style: tap-tap-hold = drag, tap-tap-release = double-click
                touchState.dragHoldTimer = setTimeout(() => {
                    // Finger still down after 150ms = start drag
                    if (!touchState.moved) {
                        touchState.isDragging = true;
                        const x = Math.round(cursorState.x);
                        const y = Math.round(cursorState.y);
                        sendMouse('down', x, y, 0);
                        const overlay = document.getElementById('cursor-overlay');
                        overlay.style.filter = 'drop-shadow(2px 2px 3px rgba(0,0,0,0.6)) hue-rotate(90deg)';
                        console.log('[Drag] Started at', x, y);
                    }
                    touchState.dragHoldTimer = null;
                }, 150);  // 150ms delay before starting drag
                touchState.lastTap = 0;  // Reset to prevent triple-tap
            } else if (controlEnabled) {
                // Single tap - set up for potential double-tap
                // Long press (500ms) for right click
                touchState.longPressTimer = setTimeout(() => {
                    if (!touchState.moved) {
                        clickAtCursor(2);  // Right click
                        showTouchIndicator(touch.clientX, touch.clientY);
                    }
                    touchState.longPressTimer = null;
                }, 500);
            }
        }, { passive: false });

        canvas.addEventListener('touchmove', function(e) {
            e.preventDefault();
            e.stopPropagation();  // Prevent exiting fullscreen

            if (e.touches.length === 2 && zoomState.isPinching) {
                // Two finger move - pinch zoom AND pan simultaneously
                const newDist = getPinchDistance(e.touches);
                const newCenter = getPinchCenter(e.touches);

                // Calculate zoom
                const delta = newDist / zoomState.lastPinchDist;
                const newScale = Math.max(zoomState.minScale, Math.min(zoomState.maxScale, zoomState.scale * delta));

                // Calculate pan (movement of pinch center)
                if (zoomState.scale > 1 || newScale > 1) {
                    const dx = newCenter.x - zoomState.pinchCenterX;
                    const dy = newCenter.y - zoomState.pinchCenterY;
                    zoomState.translateX += dx / zoomState.scale;
                    zoomState.translateY += dy / zoomState.scale;
                }

                // Update state
                zoomState.pinchCenterX = newCenter.x;
                zoomState.pinchCenterY = newCenter.y;
                zoomState.lastPinchDist = newDist;

                if (newScale !== zoomState.scale) {
                    zoomState.scale = newScale;
                    showZoomIndicator();
                }
                applyZoomTransform();
                return;
            }

            if (e.touches.length === 1 && zoomState.scale > 1 && !touchState.isDragging && !controlEnabled) {
                // Pan when zoomed (only when control mode is OFF - view-only mode)
                const touch = e.touches[0];
                const dx = touch.clientX - zoomState.pinchCenterX;
                const dy = touch.clientY - zoomState.pinchCenterY;
                // Only pan if finger moved significantly (prevents accidental pan on tap)
                if (Math.abs(dx) > 5 || Math.abs(dy) > 5) {
                    // Cancel tap detection since user is panning
                    if (touchState.longPressTimer) {
                        clearTimeout(touchState.longPressTimer);
                        touchState.longPressTimer = null;
                    }
                    zoomState.translateX += dx / zoomState.scale;
                    zoomState.translateY += dy / zoomState.scale;
                    zoomState.pinchCenterX = touch.clientX;
                    zoomState.pinchCenterY = touch.clientY;
                    applyZoomTransform();
                }
                return;
            }

            // Single finger move - touchpad mode (relative cursor movement)
            const touch = e.touches[0];
            const dx = touch.clientX - touchState.lastX;
            const dy = touch.clientY - touchState.lastY;

            // Check if moved significantly (to distinguish tap from drag)
            // Use higher threshold (20px) to allow for finger jitter during tap
            const totalDx = touch.clientX - touchState.startX;
            const totalDy = touch.clientY - touchState.startY;
            const totalDist = Math.sqrt(totalDx * totalDx + totalDy * totalDy);
            if (totalDist > 20) {
                touchState.moved = true;
                // Cancel long press if moving
                if (touchState.longPressTimer) {
                    clearTimeout(touchState.longPressTimer);
                    touchState.longPressTimer = null;
                }
                // If dragHoldTimer is pending (double-tap detected), start drag immediately on movement
                if (touchState.dragHoldTimer && !touchState.isDragging) {
                    clearTimeout(touchState.dragHoldTimer);
                    touchState.dragHoldTimer = null;
                    // Start drag now
                    touchState.isDragging = true;
                    const x = Math.round(cursorState.x);
                    const y = Math.round(cursorState.y);
                    sendMouse('down', x, y, 0);
                    const overlay = document.getElementById('cursor-overlay');
                    overlay.style.filter = 'drop-shadow(2px 2px 3px rgba(0,0,0,0.6)) hue-rotate(90deg)';
                    console.log('[Drag] Started early on movement at', x, y);
                }
            }

            // Convert screen delta to canvas delta
            // getBoundingClientRect() always reflects current visual state (including CSS transform)
            // This ensures correct calculation even during zoom transitions
            const rect = canvas.getBoundingClientRect();
            const canvasDx = dx * canvas.width / rect.width;
            const canvasDy = dy * canvas.height / rect.height;

            // Move cursor (touchpad mode)
            if (touchState.isDragging) {
                // Dragging: move cursor and send move events
                moveCursorBy(canvasDx, canvasDy);
            } else if (touchState.moved) {
                // Just moving cursor (not dragging yet)
                moveCursorBy(canvasDx, canvasDy);
            }

            touchState.lastX = touch.clientX;
            touchState.lastY = touch.clientY;
        }, { passive: false });

        canvas.addEventListener('touchend', function(e) {
            e.preventDefault();
            e.stopPropagation();  // Prevent exiting fullscreen

            // Handle pinch end
            if (zoomState.isPinching) {
                if (e.touches.length < 2) {
                    zoomState.isPinching = false;
                    // Update pan center for smooth transition to pan mode
                    if (e.touches.length === 1) {
                        zoomState.pinchCenterX = e.touches[0].clientX;
                        zoomState.pinchCenterY = e.touches[0].clientY;
                    }
                }
                touchState.touchCount = e.touches.length;
                return;
            }

            // Reset cursor style
            const overlay = document.getElementById('cursor-overlay');

            if (touchState.isDragging) {
                // End drag at cursor position
                const x = Math.round(cursorState.x);
                const y = Math.round(cursorState.y);
                sendMouse('up', x, y, 0);
                overlay.style.filter = 'drop-shadow(2px 2px 3px rgba(0,0,0,0.6))';
                console.log('[Drag] Ended at', x, y);
                touchState.lastTap = 0;  // Reset after drag
            } else if (touchState.dragHoldTimer) {
                // Double-tap but released before drag started = double click
                clearTimeout(touchState.dragHoldTimer);
                touchState.dragHoldTimer = null;
                if (!touchState.moved) {
                    console.log('[Tap] Double click');
                    dblClickAtCursor();
                }
                touchState.lastTap = 0;
            } else if (touchState.longPressTimer) {
                clearTimeout(touchState.longPressTimer);
                touchState.longPressTimer = null;
                // Tap = click at cursor position (touchpad mode)
                if (!touchState.moved) {
                    const now = Date.now();
                    // Visual feedback - flash cursor
                    overlay.style.filter = 'drop-shadow(2px 2px 3px rgba(0,0,0,0.6)) brightness(2)';
                    setTimeout(() => overlay.style.filter = 'drop-shadow(2px 2px 3px rgba(0,0,0,0.6))', 150);
                    // Single tap = left click at cursor
                    console.log('[Tap] Single click');
                    clickAtCursor(0);
                    touchState.lastTap = now;
                } else {
                    touchState.lastTap = 0;
                }
            }

            touchState.isDragging = false;
            touchState.moved = false;
            touchState.touchCount = e.touches.length;
        }, { passive: false });

        function toggleKeyboard() {
            mobileKeyboard.focus();
        }

        function sendRightClick() {
            // Use cursor position (canvas coordinates), not touch position (screen coordinates)
            const x = Math.round(cursorState.x);
            const y = Math.round(cursorState.y);
            sendMouse('down', x, y, 2);
            sendMouse('up', x, y, 2);
        }
)HTML";

    // Part 16: JavaScript - Desktop input and finalization
    html += R"HTML(
        // Desktop mouse handling
        let lastMouseMove = 0;
        let lastMouseX = 0, lastMouseY = 0;

        function getMousePos(e) {
            const rect = canvas.getBoundingClientRect();
            const scaleX = canvas.width / rect.width;
            const scaleY = canvas.height / rect.height;
            return {
                x: Math.round((e.clientX - rect.left) * scaleX),
                y: Math.round((e.clientY - rect.top) * scaleY)
            };
        }

        canvas.addEventListener('mousedown', function(e) {
            e.preventDefault();
            const pos = getMousePos(e);
            sendMouse('down', pos.x, pos.y, e.button);
        });

        canvas.addEventListener('mouseup', function(e) {
            e.preventDefault();
            const pos = getMousePos(e);
            sendMouse('up', pos.x, pos.y, e.button);
        });

        canvas.addEventListener('dblclick', function(e) {
            e.preventDefault();
            const pos = getMousePos(e);
            sendMouse('dblclick', pos.x, pos.y, e.button);
        });

        canvas.addEventListener('mousemove', function(e) {
            const now = Date.now();
            const pos = getMousePos(e);
            // Always update cursor overlay for smooth visual feedback
            updateCursorOverlay(pos.x, pos.y);
            // Throttle network sends
            const dx = Math.abs(pos.x - lastMouseX);
            const dy = Math.abs(pos.y - lastMouseY);
            const distSq = dx * dx + dy * dy;
            const minInterval = distSq > 2500 ? 33 : 50;
            if (now - lastMouseMove < minInterval && distSq < 100) return;
            lastMouseMove = now;
            lastMouseX = pos.x;
            lastMouseY = pos.y;
            sendMouse('move', pos.x, pos.y, 0);
        });

        canvas.addEventListener('wheel', function(e) {
            e.preventDefault();
            const pos = getMousePos(e);
            sendMouse('wheel', pos.x, pos.y, 0, e.deltaY);
        }, { passive: false });

        canvas.addEventListener('contextmenu', e => e.preventDefault());

        // Desktop keyboard handling
        document.addEventListener('keydown', function(e) {
            if (!document.getElementById('screen-page').classList.contains('active')) return;
            if (e.target.tagName === 'INPUT') return;
            if (e.key === 'F11') return;
            e.preventDefault();
            sendKey(e.keyCode, true, e.altKey);
        });

        document.addEventListener('keyup', function(e) {
            if (!document.getElementById('screen-page').classList.contains('active')) return;
            if (e.target.tagName === 'INPUT') return;
            if (e.key === 'F11') return;
            e.preventDefault();
            sendKey(e.keyCode, false, e.altKey);
        });

        mobileKeyboard.addEventListener('input', function(e) {
            const char = e.data;
            if (char) {
                const keyCode = char.toUpperCase().charCodeAt(0);
                sendKey(keyCode, true);
                sendKey(keyCode, false);
            }
            mobileKeyboard.value = '';
        });

        mobileKeyboard.addEventListener('keydown', function(e) {
            const keyMap = {
                'Backspace': 8, 'Tab': 9, 'Enter': 13, 'Escape': 27,
                'ArrowLeft': 37, 'ArrowUp': 38, 'ArrowRight': 39, 'ArrowDown': 40,
                'Delete': 46
            };
            if (keyMap[e.key]) {
                e.preventDefault();
                sendKey(keyMap[e.key], true);
                sendKey(keyMap[e.key], false);
            }
        });

        function disconnect() {
            // Reset control mode
            controlEnabled = false;
            // Reset floating toolbar buttons
            const btnMouse = document.getElementById('btn-mouse');
            const btnKeyboard = document.getElementById('btn-keyboard');
            if (btnMouse) btnMouse.classList.remove('active');
            if (btnKeyboard) btnKeyboard.disabled = true;
            // Reset top toolbar buttons
            const btnMouseBar = document.getElementById('btn-mouse-bar');
            const btnKeyboardBar = document.getElementById('btn-keyboard-bar');
            if (btnMouseBar) btnMouseBar.classList.remove('active');
            if (btnKeyboardBar) btnKeyboardBar.disabled = true;
            document.getElementById('screen-canvas').style.cursor = 'default';
            document.getElementById('cursor-overlay').classList.remove('active');

            // Reset zoom state
            zoomState.scale = 1;
            zoomState.translateX = 0;
            zoomState.translateY = 0;
            zoomState.isPinching = false;
            canvas.style.transform = '';
            canvas.style.transformOrigin = '';

            if (decoder) { try { decoder.close(); } catch(e) {} decoder = null; }
            if (ws && ws.readyState === WebSocket.OPEN && token) ws.send(JSON.stringify({ cmd: 'disconnect', token }));
            showPage('devices-page');
            getDevices();
        }

        function escapeHtml(text) {
            const div = document.createElement('div');
            div.textContent = text || '';
            return div.innerHTML;
        }

        function startPingInterval() {
            if (pingInterval) clearInterval(pingInterval);
            pingInterval = setInterval(() => {
                if (ws && ws.readyState === WebSocket.OPEN && token) ws.send(JSON.stringify({ cmd: 'ping', token }));
            }, 30000);
        }
        function stopPingInterval() {
            if (pingInterval) { clearInterval(pingInterval); pingInterval = null; }
        }

        document.addEventListener('keydown', (e) => {
            if (e.key === 'Enter' && document.querySelector('.page.active').id === 'login-page') login();
        });

        window.onload = function() {
            const compat = checkWebCodecs();
            if (!compat.supported) {
                document.body.insertAdjacentHTML('afterbegin',
                    '<div class="compat-warning">Warning: Your browser may not support H264 decoding.</div>');
            }
            // Restore token from sessionStorage
            token = sessionStorage.getItem('token');
            connectWebSocket();
        };
    </script>
</body>
</html>)HTML";

    return html;
}
