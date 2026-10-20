"""
Web Application for Banking System (OOP & CG Mini-Project)
Runs a zero-dependency local web server accessible via http://localhost:5000
"""

import http.server
import socketserver
import json
import urllib.parse
import datetime
import webbrowser
import threading
from banking_system import Bank, DailyLimitExceededException, InsufficientFundsException, AuthenticationException

PORT = 5000
bank = Bank()

HTML_PAGE = """<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Apex National Bank | OOP & CG Mini-Project</title>
<link href="https://fonts.googleapis.com/css2?family=Plus+Jakarta+Sans:wght@400;500;600;700;800&family=JetBrains+Mono:wght@400;500&display=swap" rel="stylesheet">
<style>
  :root {
    --bg: #090d16;
    --card: #111827;
    --card-border: #1f2937;
    --primary: #3b82f6;
    --primary-hover: #2563eb;
    --emerald: #10b981;
    --rose: #f43f5e;
    --amber: #f59e0b;
    --text: #f9fafb;
    --text-muted: #9ca3af;
  }
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body {
    font-family: 'Plus Jakarta Sans', sans-serif;
    background-color: var(--bg);
    color: var(--text);
    min-height: 100vh;
    padding-bottom: 50px;
  }
  header {
    background: linear-gradient(180deg, #111827 0%, rgba(17, 24, 39, 0.8) 100%);
    border-bottom: 1px solid var(--card-border);
    padding: 20px 32px;
    display: flex;
    justify-content: space-between;
    align-items: center;
  }
  .brand { display: flex; align-items: center; gap: 14px; }
  .brand-icon {
    width: 44px; height: 44px; border-radius: 12px;
    background: linear-gradient(135deg, #3b82f6, #06b6d4);
    display: flex; align-items: center; justify-content: center;
    font-size: 22px; font-weight: 800; color: white;
  }
  .brand-text h1 { font-size: 20px; font-weight: 800; letter-spacing: -0.5px; }
  .brand-text p { font-size: 12px; color: var(--text-muted); }
  .badge {
    background: rgba(59, 130, 246, 0.15); color: #60a5fa;
    border: 1px solid rgba(59, 130, 246, 0.3);
    padding: 6px 14px; border-radius: 9999px; font-size: 12px; font-weight: 600;
  }
  .container { max-width: 1200px; margin: 32px auto; padding: 0 24px; }
  .grid { display: grid; grid-template-columns: 340px 1fr; gap: 28px; }
  @media (max-width: 900px) { .grid { grid-template-columns: 1fr; } }
  
  .card {
    background: var(--card);
    border: 1px solid var(--card-border);
    border-radius: 16px;
    padding: 24px;
    box-shadow: 0 10px 25px -5px rgba(0, 0, 0, 0.4);
  }
  .card-title {
    font-size: 15px; font-weight: 700; color: var(--text-muted);
    text-transform: uppercase; letter-spacing: 0.5px; margin-bottom: 16px;
    display: flex; align-items: center; gap: 8px;
  }
  .menu-btn {
    width: 100%; text-align: left; padding: 12px 16px; margin-bottom: 8px;
    background: rgba(255, 255, 255, 0.03); border: 1px solid var(--card-border);
    border-radius: 10px; color: var(--text); font-weight: 600; font-size: 13.5px;
    cursor: pointer; transition: all 0.2s; display: flex; align-items: center; justify-content: space-between;
  }
  .menu-btn:hover, .menu-btn.active {
    background: rgba(59, 130, 246, 0.15); border-color: var(--primary); color: #60a5fa;
  }
  .menu-btn .key {
    background: rgba(255, 255, 255, 0.08); font-family: 'JetBrains Mono', monospace;
    padding: 2px 8px; border-radius: 6px; font-size: 11px;
  }
  .form-group { margin-bottom: 16px; }
  label { display: block; font-size: 13px; font-weight: 600; color: var(--text-muted); margin-bottom: 6px; }
  input, select {
    width: 100%; padding: 10px 14px; background: rgba(0, 0, 0, 0.3);
    border: 1px solid var(--card-border); border-radius: 10px;
    color: var(--text); font-size: 14px; font-family: inherit;
    outline: none; transition: border 0.2s;
  }
  input:focus, select:focus { border-color: var(--primary); }
  .btn-submit {
    background: var(--primary); color: white; border: none; padding: 12px 20px;
    border-radius: 10px; font-weight: 700; font-size: 14px; cursor: pointer;
    width: 100%; transition: background 0.2s; margin-top: 8px;
  }
  .btn-submit:hover { background: var(--primary-hover); }

  .output-box {
    background: #0b1120; border: 1px solid var(--card-border);
    border-radius: 12px; padding: 18px; margin-top: 20px;
    font-family: 'JetBrains Mono', monospace; font-size: 13px;
    color: #38bdf8; white-space: pre-wrap; min-height: 120px;
  }
  .canvas-container {
    background: #0b1120; border: 1px solid var(--card-border);
    border-radius: 12px; padding: 16px; margin-top: 20px; text-align: center;
  }
  canvas { background: #0b1120; border-radius: 8px; width: 100%; max-width: 720px; }
</style>
</head>
<body>

<header>
  <div class="brand">
    <div class="brand-icon">🏛️</div>
    <div class="brand-text">
      <h1>Apex National Bank</h1>
      <p>OOP & Computer Graphics Mini-Project</p>
    </div>
  </div>
  <div class="badge">Live Web App • Port 5000</div>
</header>

<div class="container">
  <div class="grid">
    <!-- Sidebar Menu -->
    <div class="card">
      <div class="card-title">Operations Menu</div>
      <button class="menu-btn active" onclick="showTab('create')"><span>a. Create Account</span><span class="key">[1]</span></button>
      <button class="menu-btn" onclick="showTab('deposit')"><span>b. Deposit Money</span><span class="key">[2]</span></button>
      <button class="menu-btn" onclick="showTab('withdraw')"><span>c. Withdraw Money</span><span class="key">[3]</span></button>
      <button class="menu-btn" onclick="showTab('daily')"><span>d. Daily Limit Status</span><span class="key">[4]</span></button>
      <button class="menu-btn" onclick="showTab('balance')"><span>e. Check Balance</span><span class="key">[5]</span></button>
      <button class="menu-btn" onclick="showTab('info')"><span>f. Account Information</span><span class="key">[6]</span></button>
      <button class="menu-btn" onclick="showTab('passbook')"><span>g. Passbook Print</span><span class="key">[7]</span></button>
      <button class="menu-btn" onclick="showTab('graphics')"><span>★ CG 2D Analytics</span><span class="key">[8]</span></button>
    </div>

    <!-- Main Workspace Area -->
    <div class="card">
      <!-- Create Account Tab -->
      <div id="tab-create" class="tab-pane">
        <div class="card-title">a. Create a New Bank Account</div>
        <div class="form-group"><label>Account Holder Full Name</label><input type="text" id="cr-name" value="Vaishnavi Jare"></div>
        <div class="form-group"><label>Mobile Phone</label><input type="text" id="cr-phone" value="+91 9876543210"></div>
        <div class="form-group"><label>Email Address</label><input type="text" id="cr-email" value="vaishnavi@sppu.edu"></div>
        <div class="form-group"><label>Account Type</label>
          <select id="cr-type">
            <option value="SAVINGS">Savings Account (Min Bal: ₹1,000 | Daily Limit: ₹25,000)</option>
            <option value="CURRENT">Current Account (Min Bal: ₹5,000 | Daily Limit: ₹1,00,000)</option>
          </select>
        </div>
        <div class="form-group"><label>4-Digit Security PIN</label><input type="password" id="cr-pin" value="1234" maxlength="4"></div>
        <div class="form-group"><label>Initial Deposit (₹)</label><input type="number" id="cr-dep" value="5000"></div>
        <button class="btn-submit" onclick="submitCreate()">Create Account Now</button>
      </div>

      <!-- Deposit Tab -->
      <div id="tab-deposit" class="tab-pane" style="display:none;">
        <div class="card-title">b. Deposit Money</div>
        <div class="form-group"><label>Account Number</label><input type="text" id="dep-acc" value="SB1001"></div>
        <div class="form-group"><label>Deposit Amount (₹)</label><input type="number" id="dep-amt" value="2000"></div>
        <div class="form-group"><label>Remarks / Description</label><input type="text" id="dep-desc" value="Cash Deposit"></div>
        <button class="btn-submit" onclick="submitDeposit()">Confirm Deposit</button>
      </div>

      <!-- Withdraw Tab -->
      <div id="tab-withdraw" class="tab-pane" style="display:none;">
        <div class="card-title">c. Withdraw Money (Honoring Daily Limit)</div>
        <div class="form-group"><label>Account Number</label><input type="text" id="wth-acc" value="SB1001"></div>
        <div class="form-group"><label>4-Digit Security PIN</label><input type="password" id="wth-pin" value="1234"></div>
        <div class="form-group"><label>Withdrawal Amount (₹)</label><input type="number" id="wth-amt" value="3000"></div>
        <button class="btn-submit" onclick="submitWithdraw()">Authorize & Withdraw</button>
      </div>

      <!-- Daily Limit Tab -->
      <div id="tab-daily" class="tab-pane" style="display:none;">
        <div class="card-title">d. Honor Daily Withdrawal Limit Status</div>
        <div class="form-group"><label>Account Number</label><input type="text" id="dl-acc" value="SB1001"></div>
        <div class="form-group"><label>4-Digit Security PIN</label><input type="password" id="dl-pin" value="1234"></div>
        <button class="btn-submit" onclick="submitDailyLimit()">Check Daily Limit Status</button>
      </div>

      <!-- Balance Tab -->
      <div id="tab-balance" class="tab-pane" style="display:none;">
        <div class="card-title">e. Check Account Balance</div>
        <div class="form-group"><label>Account Number</label><input type="text" id="bal-acc" value="SB1001"></div>
        <div class="form-group"><label>4-Digit Security PIN</label><input type="password" id="bal-pin" value="1234"></div>
        <button class="btn-submit" onclick="submitBalance()">Inquire Balance</button>
      </div>

      <!-- Info Tab -->
      <div id="tab-info" class="tab-pane" style="display:none;">
        <div class="card-title">f. Display Account Information</div>
        <div class="form-group"><label>Account Number</label><input type="text" id="info-acc" value="SB1001"></div>
        <button class="btn-submit" onclick="submitInfo()">Display Customer Details</button>
      </div>

      <!-- Passbook Tab -->
      <div id="tab-passbook" class="tab-pane" style="display:none;">
        <div class="card-title">g. Passbook Print (From Date to To Date)</div>
        <div class="form-group"><label>Account Number</label><input type="text" id="pb-acc" value="SB1001"></div>
        <div class="form-group"><label>4-Digit PIN</label><input type="password" id="pb-pin" value="1234"></div>
        <div class="form-group"><label>From Date (YYYY-MM-DD)</label><input type="date" id="pb-from" value="2026-01-01"></div>
        <div class="form-group"><label>To Date (YYYY-MM-DD)</label><input type="date" id="pb-to" value="2026-12-31"></div>
        <button class="btn-submit" onclick="submitPassbook()">Generate Official Passbook</button>
      </div>

      <!-- Graphics Tab -->
      <div id="tab-graphics" class="tab-pane" style="display:none;">
        <div class="card-title">★ Computer Graphics (CG) 2D Visual Analytics</div>
        <div class="form-group"><label>Account Number</label><input type="text" id="cg-acc" value="SB1001"></div>
        <button class="btn-submit" onclick="renderCanvasGraphics()">Render 2D Graphics Canvas</button>
        <div class="canvas-container">
          <canvas id="cgCanvas" width="700" height="380"></canvas>
        </div>
      </div>

      <div class="output-box" id="output">Ready. Select an operation to execute.</div>
    </div>
  </div>
</div>

<script>
function showTab(id) {
  document.querySelectorAll('.tab-pane').forEach(el => el.style.display = 'none');
  document.querySelectorAll('.menu-btn').forEach(el => el.classList.remove('active'));
  document.getElementById('tab-' + id).style.display = 'block';
  event.currentTarget.classList.add('active');
  if (id === 'graphics') renderCanvasGraphics();
}

async function apiCall(action, params) {
  const query = new URLSearchParams(params).toString();
  const res = await fetch('/api/' + action + '?' + query);
  const data = await res.json();
  const out = document.getElementById('output');
  if (data.status === 'success') {
    out.style.color = '#38bdf8';
    out.textContent = data.message || JSON.stringify(data, null, 2);
  } else {
    out.style.color = '#f43f5e';
    out.textContent = '[ERROR] ' + (data.message || 'Operation failed');
  }
  return data;
}

function submitCreate() {
  apiCall('create', {
    name: document.getElementById('cr-name').value,
    phone: document.getElementById('cr-phone').value,
    email: document.getElementById('cr-email').value,
    type: document.getElementById('cr-type').value,
    pin: document.getElementById('cr-pin').value,
    initial: document.getElementById('cr-dep').value
  });
}

function submitDeposit() {
  apiCall('deposit', {
    acc: document.getElementById('dep-acc').value,
    amount: document.getElementById('dep-amt').value,
    desc: document.getElementById('dep-desc').value
  });
}

function submitWithdraw() {
  apiCall('withdraw', {
    acc: document.getElementById('wth-acc').value,
    pin: document.getElementById('wth-pin').value,
    amount: document.getElementById('wth-amt').value
  });
}

function submitDailyLimit() {
  apiCall('daily_limit', {
    acc: document.getElementById('dl-acc').value,
    pin: document.getElementById('dl-pin').value
  });
}

function submitBalance() {
  apiCall('balance', {
    acc: document.getElementById('bal-acc').value,
    pin: document.getElementById('bal-pin').value
  });
}

function submitInfo() {
  apiCall('info', { acc: document.getElementById('info-acc').value });
}

function submitPassbook() {
  apiCall('passbook', {
    acc: document.getElementById('pb-acc').value,
    pin: document.getElementById('pb-pin').value,
    from: document.getElementById('pb-from').value,
    to: document.getElementById('pb-to').value
  });
}

async function renderCanvasGraphics() {
  const accNo = document.getElementById('cg-acc').value;
  const data = await apiCall('analytics_data', { acc: accNo });
  if (data.status !== 'success') return;

  const canvas = document.getElementById('cgCanvas');
  const ctx = canvas.getContext('2d');
  ctx.clearRect(0, 0, canvas.width, canvas.height);

  // Background grid
  ctx.fillStyle = '#0b1120';
  ctx.fillRect(0, 0, canvas.width, canvas.height);

  // Title
  ctx.fillStyle = '#f8fafc';
  ctx.font = 'bold 15px Plus Jakarta Sans';
  ctx.fillText(data.acc_no + ' (' + data.holder + ') - 2D Graphics Canvas', 20, 30);

  // Daily limit gauge
  ctx.font = '12px Plus Jakarta Sans';
  ctx.fillStyle = '#94a3b8';
  ctx.fillText('Daily Limit Meter: Used ₹' + data.withdrawn_today + ' / ₹' + data.daily_limit + ' (' + data.pct_used + '%)', 20, 60);

  // Progress bar
  ctx.fillStyle = '#1e293b';
  ctx.fillRect(20, 72, 660, 16);
  ctx.fillStyle = data.pct_used > 85 ? '#ef4444' : (data.pct_used > 50 ? '#f59e0b' : '#10b981');
  ctx.fillRect(20, 72, 660 * Math.min(1.0, data.pct_used / 100), 16);

  // 2D Bar Chart
  const txns = data.transactions;
  if (!txns || txns.length === 0) return;

  const plotX0 = 60, plotY0 = 330, plotX1 = 660, plotY1 = 130;
  ctx.strokeStyle = '#334155';
  ctx.lineWidth = 1;
  ctx.beginPath();
  ctx.moveTo(plotX0, plotY1); ctx.lineTo(plotX0, plotY0); ctx.lineTo(plotX1, plotY0);
  ctx.stroke();

  let maxVal = 10000;
  txns.forEach(t => { if (t.amount > maxVal) maxVal = t.amount; if (t.balance_after > maxVal) maxVal = t.balance_after; });

  const stepX = (plotX1 - plotX0) / (txns.length + 1);
  const balPts = [];

  txns.forEach((t, i) => {
    const cx = plotX0 + (i + 1) * stepX;
    const barH = (t.amount / maxVal) * (plotY0 - plotY1);
    ctx.fillStyle = (t.type === 'WITHDRAWAL') ? '#ef4444' : '#10b981';
    ctx.fillRect(cx - 12, plotY0 - barH, 24, barH);

    ctx.fillStyle = '#94a3b8';
    ctx.font = '10px JetBrains Mono';
    ctx.fillText(t.id, cx - 14, plotY0 + 16);

    const balY = plotY0 - (t.balance_after / maxVal) * (plotY0 - plotY1);
    balPts.push({ x: cx, y: balY });
  });

  // Balance Polyline
  if (balPts.length > 1) {
    ctx.strokeStyle = '#3b82f6';
    ctx.lineWidth = 3;
    ctx.beginPath();
    ctx.moveTo(balPts[0].x, balPts[0].y);
    for (let i = 1; i < balPts.length; i++) ctx.lineTo(balPts[i].x, balPts[i].y);
    ctx.stroke();

    balPts.forEach(pt => {
      ctx.fillStyle = '#3b82f6';
      ctx.beginPath();
      ctx.arc(pt.x, pt.y, 4, 0, Math.PI * 2);
      ctx.fill();
    });
  }
}
</script>
</body>
</html>
"""

class BankingHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        url = urllib.parse.urlparse(self.path)
        if url.path == "/" or url.path == "/index.html":
            self.send_response(200)
            self.send_header("Content-type", "text/html; charset=utf-8")
            self.end_headers()
            self.wfile.write(HTML_PAGE.encode("utf-8"))
            return

        if url.path.startswith("/api/"):
            action = url.path[len("/api/"):]
            params = dict(urllib.parse.parse_qsl(url.query))
            response = {}

            try:
                if action == "create":
                    acc_no = bank.create_account(
                        params.get("name", ""),
                        params.get("phone", ""),
                        params.get("email", ""),
                        params.get("type", "SAVINGS"),
                        params.get("pin", "1234"),
                        float(params.get("initial", 1000))
                    )
                    response = {"status": "success", "message": f"Account Created Successfully! Account Number: {acc_no}"}

                elif action == "deposit":
                    acc_no = params.get("acc")
                    amt = float(params.get("amount", 0))
                    desc = params.get("desc", "Deposit")
                    bank.deposit(acc_no, amt, desc)
                    acc = bank.get_account(acc_no)
                    response = {"status": "success", "message": f"Successfully Deposited INR {amt:,.2f}.\nUpdated Balance: INR {acc._balance:,.2f}"}

                elif action == "withdraw":
                    acc_no = params.get("acc")
                    pin = params.get("pin")
                    amt = float(params.get("amount", 0))
                    bank.withdraw(acc_no, pin, amt, "Cash Withdrawal")
                    acc = bank.get_account(acc_no)
                    response = {"status": "success", "message": f"TRANSACTION APPROVED!\nDispensed: INR {amt:,.2f}\nUpdated Balance: INR {acc._balance:,.2f}\nRemaining Daily Limit: INR {acc.get_remaining_daily_limit():,.2f}"}

                elif action == "daily_limit":
                    acc_no = params.get("acc")
                    pin = params.get("pin")
                    status = bank.honor_daily_limit(acc_no, pin)
                    acc = bank.get_account(acc_no)
                    msg = (
                        f"DAILY WITHDRAWAL LIMIT STATUS: {acc_no} ({acc._holder_name})\n"
                        f"  * Configured Daily Limit : INR {status['daily_limit']:,.2f}\n"
                        f"  * Withdrawn Today        : INR {status['withdrawn_today']:,.2f}\n"
                        f"  * Remaining Allowance    : INR {status['remaining_limit']:,.2f}\n"
                        f"  * Last Withdrawal Date   : {status['last_date']}"
                    )
                    response = {"status": "success", "message": msg}

                elif action == "balance":
                    acc_no = params.get("acc")
                    pin = params.get("pin")
                    bal = bank.check_balance(acc_no, pin)
                    acc = bank.get_account(acc_no)
                    msg = (
                        f"BALANCE ENQUIRY REPORT\n"
                        f"  * Account Number  : {acc_no}\n"
                        f"  * Holder Name     : {acc._holder_name}\n"
                        f"  * Account Type    : {acc.get_account_type()}\n"
                        f"  * Current Balance : INR {bal:,.2f}\n"
                        f"  * Minimum Balance : INR {acc.get_minimum_balance():,.2f}\n"
                        f"  * Remaining Daily Allowance: INR {acc.get_remaining_daily_limit():,.2f}"
                    )
                    response = {"status": "success", "message": msg}

                elif action == "info":
                    acc_no = params.get("acc")
                    acc = bank.get_account(acc_no)
                    response = {"status": "success", "message": acc.display_account_info()}

                elif action == "passbook":
                    acc_no = params.get("acc")
                    pin = params.get("pin")
                    acc = bank.get_account(acc_no)
                    if not acc.authenticate(pin):
                        raise AuthenticationException()
                    from_d = datetime.date.fromisoformat(params.get("from", "1970-01-01"))
                    to_d = datetime.date.fromisoformat(params.get("to", str(datetime.date.today())))
                    pb_text = acc.generate_passbook_text(from_d, to_d)
                    response = {"status": "success", "message": pb_text}

                elif action == "analytics_data":
                    acc_no = params.get("acc")
                    acc = bank.get_account(acc_no)
                    acc.check_daily_limit_reset()
                    limit = acc._daily_withdrawal_limit
                    wth = acc._withdrawn_today
                    pct = int((wth / limit) * 100) if limit > 0 else 0
                    txns = [
                        {"id": t.txn_id, "type": t.txn_type, "amount": t.amount, "balance_after": t.balance_after}
                        for t in acc._transactions[-8:]
                    ]
                    response = {
                        "status": "success",
                        "acc_no": acc._account_number,
                        "holder": acc._holder_name,
                        "balance": acc._balance,
                        "daily_limit": limit,
                        "withdrawn_today": wth,
                        "pct_used": pct,
                        "transactions": txns
                    }

            except DailyLimitExceededException as de:
                response = {"status": "error", "message": str(de)}
            except Exception as e:
                response = {"status": "error", "message": str(e)}

            self.send_response(200)
            self.send_header("Content-type", "application/json")
            self.end_headers()
            self.wfile.write(json.dumps(response).encode("utf-8"))
            return

        self.send_response(404)
        self.end_headers()

def open_browser(port):
    try:
        webbrowser.open(f"http://localhost:{port}")
    except Exception:
        pass

def run_server():
    socketserver.TCPServer.allow_reuse_address = True
    with socketserver.TCPServer(("", PORT), BankingHandler) as httpd:
        print(f"Serving Banking System on http://localhost:{PORT}")
        print(f"Opening your browser automatically at http://localhost:{PORT} ...")
        threading.Timer(0.5, open_browser, args=[PORT]).start()
        httpd.serve_forever()

if __name__ == "__main__":
    run_server()
