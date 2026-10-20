"""
OOP & Computer Graphics (CG) Mini-Project: Modern Banking System
Author: Vaishnavi Jare (vaishnavijare60-hash)
Curriculum Alignment: Object Oriented Programming & Computer Graphics

Features Implemented:
  a. Create an account (Savings & Current)
  b. Deposit money
  c. Withdraw money
  d. Honor daily withdrawal limit
  e. Check the balance
  f. Display Account information
  g. Passbook Print (from date/txn to date/txn)
  + 2D Computer Graphics Analytics Dashboard (Bar Chart & Balance Trend Curve)
"""

import sys
import os
import datetime
from abc import ABC, abstractmethod
try:
    import tkinter as tk
    from tkinter import messagebox, ttk
except ImportError:
    tk = messagebox = ttk = None

# ==============================================================================
# 1. Custom Exceptions (OOP Exception Hierarchy)
# ==============================================================================

class BankException(Exception):
    """Base class for all banking exceptions."""
    pass

class AccountNotFoundException(BankException):
    def __init__(self, acc_no):
        super().__init__(f"Account Not Found: Account Number '{acc_no}' does not exist.")

class InsufficientFundsException(BankException):
    def __init__(self, requested, current, min_balance):
        super().__init__(
            f"Insufficient Funds: Requested withdrawal of INR {requested:,.2f} "
            f"exceeds available funds. (Current: INR {current:,.2f}, Min Balance: INR {min_balance:,.2f})"
        )
        self.requested = requested
        self.current = current
        self.min_balance = min_balance

class DailyLimitExceededException(BankException):
    def __init__(self, requested, withdrawn_today, daily_limit):
        remaining = max(0.0, daily_limit - withdrawn_today)
        msg = (
            f"Daily Withdrawal Limit Exceeded!\n"
            f"  * Daily Limit Allowed : INR {daily_limit:,.2f}\n"
            f"  * Already Withdrawn   : INR {withdrawn_today:,.2f}\n"
            f"  * Remaining Today     : INR {remaining:,.2f}\n"
            f"  * Attempted Amount    : INR {requested:,.2f}"
        )
        super().__init__(msg)
        self.requested = requested
        self.withdrawn_today = withdrawn_today
        self.daily_limit = daily_limit
        self.remaining = remaining

class InvalidAmountException(BankException):
    pass

class AuthenticationException(BankException):
    def __init__(self):
        super().__init__("Authentication Failed: Incorrect 4-digit PIN provided.")


# ==============================================================================
# 2. Transaction Class (Encapsulation & Data Modeling)
# ==============================================================================

class Transaction:
    def __init__(self, txn_id, timestamp, txn_type, amount, balance_after, description=""):
        self.txn_id = txn_id
        self.timestamp = timestamp if isinstance(timestamp, datetime.datetime) else datetime.datetime.fromisoformat(timestamp)
        self.txn_type = txn_type  # 'OPENING', 'DEPOSIT', 'WITHDRAWAL', 'INTEREST'
        self.amount = float(amount)
        self.balance_after = float(balance_after)
        self.description = description

    def serialize(self):
        return f"{self.txn_id}|{self.timestamp.strftime('%Y-%m-%d %H:%M:%S')}|{self.txn_type}|{self.amount:.2f}|{self.balance_after:.2f}|{self.description}"

    @classmethod
    def deserialize(cls, line):
        parts = line.strip().split('|')
        if len(parts) >= 6:
            dt = datetime.datetime.strptime(parts[1], "%Y-%m-%d %H:%M:%S")
            return cls(parts[0], dt, parts[2], float(parts[3]), float(parts[4]), parts[5])
        return None

    def __str__(self):
        return (f"[{self.timestamp.strftime('%Y-%m-%d %H:%M:%S')}] {self.txn_id} | "
                f"{self.txn_type:11} | INR {self.amount:10.2f} | Bal: INR {self.balance_after:10.2f} | {self.description}")


# ==============================================================================
# 3. Account Abstract Base Class (Abstraction, Encapsulation, Polymorphism)
# ==============================================================================

class Account(ABC):
    def __init__(self, acc_no, holder_name, phone, email, pin, initial_balance, daily_limit):
        self._account_number = acc_no
        self._holder_name = holder_name
        self._phone = phone
        self._email = email
        self._pin = str(pin)
        self._balance = float(initial_balance)
        self._creation_date = datetime.datetime.now()
        self._daily_withdrawal_limit = float(daily_limit)
        self._withdrawn_today = 0.0
        self._last_withdrawal_date = datetime.date.today()
        self._transactions = []
        self._next_txn_id = 1001

        if initial_balance > 0:
            self._transactions.append(
                Transaction(self._generate_txn_id(), self._creation_date,
                            "OPENING", initial_balance, self._balance, "Account Opening Initial Deposit")
            )

    def _generate_txn_id(self):
        tid = f"TXN{self._next_txn_id}"
        self._next_txn_id += 1
        return tid

    # --- Pure Virtual / Abstract Methods ---
    @abstractmethod
    def get_account_type(self) -> str:
        pass

    @abstractmethod
    def get_minimum_balance(self) -> float:
        pass

    @abstractmethod
    def get_interest_rate(self) -> float:
        pass

    @abstractmethod
    def apply_interest(self):
        pass

    # --- Authentication ---
    def authenticate(self, input_pin: str) -> bool:
        return self._pin == str(input_pin).strip()

    # --- Daily Limit Handling (Requirement d) ---
    def check_daily_limit_reset(self):
        today = datetime.date.today()
        if today != self._last_withdrawal_date:
            self._withdrawn_today = 0.0
            self._last_withdrawal_date = today

    def get_remaining_daily_limit(self) -> float:
        self.check_daily_limit_reset()
        return max(0.0, self._daily_withdrawal_limit - self._withdrawn_today)

    def set_daily_withdrawal_limit(self, new_limit: float):
        if new_limit < 0:
            raise InvalidAmountException("Daily withdrawal limit cannot be negative.")
        self._daily_withdrawal_limit = float(new_limit)

    # --- Core Operations (b, c, e, f, g) ---
    def deposit(self, amount: float, description: str = "Cash Deposit"):
        if amount <= 0:
            raise InvalidAmountException("Deposit amount must be strictly greater than zero.")
        self._balance += amount
        self._transactions.append(
            Transaction(self._generate_txn_id(), datetime.datetime.now(),
                        "DEPOSIT", amount, self._balance, description)
        )

    def withdraw(self, amount: float, input_pin: str, description: str = "Cash Withdrawal"):
        if not self.authenticate(input_pin):
            raise AuthenticationException()
        if amount <= 0:
            raise InvalidAmountException("Withdrawal amount must be strictly greater than zero.")

        self.check_daily_limit_reset()

        # Requirement d: Honor daily withdrawal limit
        if self._withdrawn_today + amount > self._daily_withdrawal_limit:
            raise DailyLimitExceededException(amount, self._withdrawn_today, self._daily_withdrawal_limit)

        # Minimum Balance enforcement
        if self._balance - amount < self.get_minimum_balance():
            raise InsufficientFundsException(amount, self._balance, self.get_minimum_balance())

        self._balance -= amount
        self._withdrawn_today += amount
        self._last_withdrawal_date = datetime.date.today()

        self._transactions.append(
            Transaction(self._generate_txn_id(), datetime.datetime.now(),
                        "WITHDRAWAL", amount, self._balance, description)
        )

    def get_balance(self, input_pin: str) -> float:
        if not self.authenticate(input_pin):
            raise AuthenticationException()
        return self._balance

    def display_account_info(self) -> str:
        self.check_daily_limit_reset()
        rem_limit = self.get_remaining_daily_limit()
        lines = [
            "=" * 64,
            "                   ACCOUNT INFORMATION DETAILS                  ",
            "=" * 64,
            f"{'Account Number':<26}: {self._account_number}",
            f"{'Account Type':<26}: {self.get_account_type()}",
            f"{'Account Holder':<26}: {self._holder_name}",
            f"{'Contact Phone':<26}: {self._phone}",
            f"{'Email Address':<26}: {self._email}",
            f"{'Created On':<26}: {self._creation_date.strftime('%Y-%m-%d %H:%M:%S')}",
            "-" * 64,
            f"{'Current Balance':<26}: INR {self._balance:,.2f}",
            f"{'Minimum Balance':<26}: INR {self.get_minimum_balance():,.2f}",
            f"{'Annual Interest Rate':<26}: {self.get_interest_rate()}%",
            "-" * 64,
            f"{'Daily Withdrawal Limit':<26}: INR {self._daily_withdrawal_limit:,.2f}",
            f"{'Total Withdrawn Today':<26}: INR {self._withdrawn_today:,.2f}",
            f"{'Remaining Daily Limit':<26}: INR {rem_limit:,.2f}",
            f"{'Total Transactions':<26}: {len(self._transactions)}",
            "=" * 64,
        ]
        return "\n".join(lines)

    def generate_passbook_text(self, from_date: datetime.date, to_date: datetime.date) -> str:
        lines = [
            "=" * 92,
            "                                OFFICIAL BANK PASSBOOK                                  ",
            "=" * 92,
            f"Account Number : {self._account_number:<16} Holder Name  : {self._holder_name}",
            f"Account Type   : {self.get_account_type():<16} Filter Range : {from_date} to {to_date}",
            f"Current Bal    : INR {self._balance:,.2f}",
            "-" * 92,
            f"{'Date & Time':<20}{'Txn ID':<11}{'Type':<12}{'Debit (INR)':<14}{'Credit (INR)':<14}{'Balance (INR)':<16}{'Description'}",
            "-" * 92,
        ]

        total_debit = 0.0
        total_credit = 0.0
        matched = 0

        for t in self._transactions:
            t_date = t.timestamp.date()
            if from_date <= t_date <= to_date:
                matched += 1
                if t.txn_type == "WITHDRAWAL":
                    debit_str = f"{t.amount:,.2f}"
                    credit_str = "-"
                    total_debit += t.amount
                else:
                    debit_str = "-"
                    credit_str = f"{t.amount:,.2f}"
                    total_credit += t.amount

                lines.append(
                    f"{t.timestamp.strftime('%Y-%m-%d %H:%M:%S'):<20}"
                    f"{t.txn_id:<11}"
                    f"{t.txn_type:<12}"
                    f"{debit_str:<14}"
                    f"{credit_str:<14}"
                    f"{t.balance_after:,.2f}{'':<6}"
                    f"{t.description}"
                )

        if matched == 0:
            lines.append("              No transactions found within the specified date range.            ")

        lines.extend([
            "-" * 92,
            "SUMMARY FOR PERIOD:",
            f"  * Total Transactions Recorded : {matched}",
            f"  * Total Amount Debited        : INR {total_debit:,.2f}",
            f"  * Total Amount Credited       : INR {total_credit:,.2f}",
            f"  * Net Transaction Difference  : INR {(total_credit - total_debit):,.2f}",
            "=" * 92,
        ])
        return "\n".join(lines)


# ==============================================================================
# 4. Derived Classes: SavingsAccount and CurrentAccount (Inheritance & Polymorphism)
# ==============================================================================

class SavingsAccount(Account):
    MINIMUM_BALANCE = 1000.0
    INTEREST_RATE = 4.0
    DEFAULT_DAILY_LIMIT = 25000.0

    def __init__(self, acc_no, holder_name, phone, email, pin, initial_balance, daily_limit=None):
        limit = float(daily_limit) if (daily_limit and float(daily_limit) > 0) else self.DEFAULT_DAILY_LIMIT
        super().__init__(acc_no, holder_name, phone, email, pin, initial_balance, limit)

    def get_account_type(self) -> str:
        return "SAVINGS"

    def get_minimum_balance(self) -> float:
        return self.MINIMUM_BALANCE

    def get_interest_rate(self) -> float:
        return self.INTEREST_RATE

    def apply_interest(self):
        interest = (self._balance * self.INTEREST_RATE) / 100.0
        if interest > 0:
            self._balance += interest
            self._transactions.append(
                Transaction(self._generate_txn_id(), datetime.datetime.now(),
                            "INTEREST", interest, self._balance, f"Annual Interest Credit @ {self.INTEREST_RATE}%")
            )


class CurrentAccount(Account):
    MINIMUM_BALANCE = 5000.0
    INTEREST_RATE = 0.0
    DEFAULT_DAILY_LIMIT = 100000.0

    def __init__(self, acc_no, holder_name, phone, email, pin, initial_balance, daily_limit=None):
        limit = float(daily_limit) if (daily_limit and float(daily_limit) > 0) else self.DEFAULT_DAILY_LIMIT
        super().__init__(acc_no, holder_name, phone, email, pin, initial_balance, limit)

    def get_account_type(self) -> str:
        return "CURRENT"

    def get_minimum_balance(self) -> float:
        return self.MINIMUM_BALANCE

    def get_interest_rate(self) -> float:
        return self.INTEREST_RATE

    def apply_interest(self):
        pass  # Current accounts do not earn interest


# ==============================================================================
# 5. Bank Class (Business Logic & Account Management)
# ==============================================================================

class Bank:
    def __init__(self, name="APEX NATIONAL BANK", data_file="data/bank_data.txt"):
        self.bank_name = name
        self.data_file = data_file
        self.accounts = {}
        self.next_savings_seq = 1001
        self.next_current_seq = 2001
        self.load_sample_data()

    def generate_account_number(self, acc_type: str) -> str:
        if acc_type.upper().startswith("CURR") or acc_type.upper() == "CA":
            acc_no = f"CA{self.next_current_seq}"
            self.next_current_seq += 1
        else:
            acc_no = f"SB{self.next_savings_seq}"
            self.next_savings_seq += 1
        return acc_no

    def create_account(self, name, phone, email, acc_type, pin, initial_deposit, custom_limit=0.0):
        if len(str(pin)) != 4 or not str(pin).isdigit():
            raise BankException("PIN must be an exact 4-digit numeric code.")

        acc_type_upper = acc_type.upper()
        if "CURR" in acc_type_upper or acc_type_upper == "CA":
            if initial_deposit < CurrentAccount.MINIMUM_BALANCE:
                raise InvalidAmountException(f"Current Account requires min initial deposit of INR {CurrentAccount.MINIMUM_BALANCE:,.2f}")
            acc_no = self.generate_account_number("CA")
            acc = CurrentAccount(acc_no, name, phone, email, pin, initial_deposit, custom_limit)
        else:
            if initial_deposit < SavingsAccount.MINIMUM_BALANCE:
                raise InvalidAmountException(f"Savings Account requires min initial deposit of INR {SavingsAccount.MINIMUM_BALANCE:,.2f}")
            acc_no = self.generate_account_number("SB")
            acc = SavingsAccount(acc_no, name, phone, email, pin, initial_deposit, custom_limit)

        self.accounts[acc_no] = acc
        return acc_no

    def get_account(self, acc_no: str) -> Account:
        if acc_no not in self.accounts:
            raise AccountNotFoundException(acc_no)
        return self.accounts[acc_no]

    def deposit(self, acc_no: str, amount: float, desc: str = "Cash Deposit"):
        acc = self.get_account(acc_no)
        acc.deposit(amount, desc)

    def withdraw(self, acc_no: str, pin: str, amount: float, desc: str = "Cash Withdrawal"):
        acc = self.get_account(acc_no)
        acc.withdraw(amount, pin, desc)

    def check_balance(self, acc_no: str, pin: str) -> float:
        acc = self.get_account(acc_no)
        return acc.get_balance(pin)

    def honor_daily_limit(self, acc_no: str, pin: str, test_amount: float = 0.0) -> dict:
        acc = self.get_account(acc_no)
        if not acc.authenticate(pin):
            raise AuthenticationException()
        acc.check_daily_limit_reset()
        rem = acc.get_remaining_daily_limit()
        if test_amount > 0 and test_amount > rem:
            raise DailyLimitExceededException(test_amount, acc._withdrawn_today, acc._daily_withdrawal_limit)
        return {
            "daily_limit": acc._daily_withdrawal_limit,
            "withdrawn_today": acc._withdrawn_today,
            "remaining_limit": rem,
            "last_date": acc._last_withdrawal_date
        }

    def load_sample_data(self):
        acc1 = self.create_account("Vaishnavi Jare", "+91 9876543210", "vaishnavi@sppu.edu", "SAVINGS", "1234", 25000.0, 20000.0)
        self.deposit(acc1, 5000.0, "Salary Credited")
        self.deposit(acc1, 2500.0, "Freelance Payment")
        self.withdraw(acc1, "1234", 3000.0, "ATM Cash Withdrawal")
        self.withdraw(acc1, "1234", 1500.0, "Grocery Store POS")

        acc2 = self.create_account("Tech Innovations Corp", "+91 9123456780", "accounts@techcorp.in", "CURRENT", "9999", 75000.0, 50000.0)
        self.deposit(acc2, 20000.0, "Client Invoice Payment")
        self.withdraw(acc2, "9999", 10000.0, "Vendor Settlement")


# ==============================================================================
# 6. Computer Graphics (CG) Visualization Module: 2D Canvas Dashboard
# ==============================================================================

class CGVisualizer:
    @staticmethod
    def show_dashboard(account: Account):
        if tk is None:
            print("\n[!] 2D Graphical Dashboard requires tkinter, which is not installed on this system.")
            return
        win = tk.Toplevel()
        win.title(f"2D Computer Graphics Analytics | {account._account_number} ({account._holder_name})")
        win.geometry("960x700")
        win.configure(bg="#0F172A")

        canvas = tk.Canvas(win, width=960, height=700, bg="#F8FAFC", highlightthickness=0)
        canvas.pack(fill="both", expand=True)

        # Header banner
        canvas.create_rectangle(0, 0, 960, 90, fill="#0F172A", outline="")

        # Bank vector icon
        # Roof triangle
        canvas.create_polygon(30, 48, 55, 22, 80, 48, fill="#38BDF8", outline="white", width=2)
        # Pediment
        canvas.create_rectangle(32, 48, 78, 53, fill="#38BDF8", outline="white", width=1)
        # Pillars
        for px in [35, 46, 57, 68]:
            canvas.create_rectangle(px, 54, px + 7, 72, fill="white", outline="")
        # Base
        canvas.create_rectangle(30, 72, 80, 77, fill="#38BDF8", outline="white", width=1)

        # Title
        canvas.create_text(100, 32, text="APEX NATIONAL BANK - COMPUTER GRAPHICS ANALYTICS",
                           font=("Segoe UI", 16, "bold"), fill="white", anchor="w")
        canvas.create_text(100, 58, text="2D Graphical Dashboard & Transaction Visualizer | OOP & CG Project",
                           font=("Segoe UI", 10), fill="#94A3B8", anchor="w")

        # Account Info Card
        canvas.create_rectangle(30, 105, 930, 215, fill="white", outline="#E2E8F0", width=1)

        canvas.create_text(50, 125, text="ACCOUNT HOLDER", font=("Segoe UI", 8), fill="#64748B", anchor="w")
        canvas.create_text(50, 145, text=account._holder_name, font=("Segoe UI", 12, "bold"), fill="#0F172A", anchor="w")

        canvas.create_text(260, 125, text="ACCOUNT NUMBER", font=("Segoe UI", 8), fill="#64748B", anchor="w")
        canvas.create_text(260, 145, text=account._account_number, font=("Segoe UI", 12, "bold"), fill="#0284C7", anchor="w")

        canvas.create_text(440, 125, text="ACCOUNT TYPE", font=("Segoe UI", 8), fill="#64748B", anchor="w")
        canvas.create_text(440, 145, text=f"{account.get_account_type()} ACCOUNT", font=("Segoe UI", 12, "bold"), fill="#0F172A", anchor="w")

        canvas.create_text(680, 125, text="CURRENT BALANCE", font=("Segoe UI", 8), fill="#64748B", anchor="w")
        canvas.create_text(680, 145, text=f"INR {account._balance:,.2f}", font=("Segoe UI", 15, "bold"), fill="#10B981", anchor="w")

        # Daily Withdrawal Limit Meter (Requirement d)
        account.check_daily_limit_reset()
        rem_limit = account.get_remaining_daily_limit()
        ratio = account._withdrawn_today / account._daily_withdrawal_limit if account._daily_withdrawal_limit > 0 else 0
        ratio = min(1.0, ratio)

        canvas.create_text(50, 185, text="Daily Limit Gauge:", font=("Segoe UI", 9, "bold"), fill="#475569", anchor="w")

        meter_x, meter_y, meter_w, meter_h = 175, 175, 380, 20
        canvas.create_rectangle(meter_x, meter_y, meter_x + meter_w, meter_y + meter_h, fill="#E2E8F0", outline="#CBD5E1")

        color = "#22C55E" if ratio < 0.50 else ("#F59E0B" if ratio < 0.85 else "#EF4444")
        fill_w = int(ratio * meter_w)
        if fill_w > 0:
            canvas.create_rectangle(meter_x, meter_y, meter_x + fill_w, meter_y + meter_h, fill=color, outline="")

        canvas.create_text(meter_x + meter_w + 15, 185,
                           text=f"Used: INR {account._withdrawn_today:,.0f} / {account._daily_withdrawal_limit:,.0f} ({int(ratio*100)}%) | Remaining: INR {rem_limit:,.0f}",
                           font=("Segoe UI", 9), fill="#334155", anchor="w")

        # 2D Chart Area (Bar Chart + Line Curve)
        canvas.create_rectangle(30, 230, 930, 650, fill="white", outline="#E2E8F0", width=1)
        canvas.create_text(50, 252, text="2D Computer Graphics: Transaction Distribution & Balance Trajectory",
                           font=("Segoe UI", 12, "bold"), fill="#0F172A", anchor="w")

        # Legend
        canvas.create_rectangle(570, 246, 582, 258, fill="#10B981", outline="")
        canvas.create_text(588, 252, text="Deposit (+)", font=("Segoe UI", 8), fill="#475569", anchor="w")

        canvas.create_rectangle(675, 246, 687, 258, fill="#EF4444", outline="")
        canvas.create_text(693, 252, text="Withdrawal (-)", font=("Segoe UI", 8), fill="#475569", anchor="w")

        canvas.create_line(790, 252, 810, 252, fill="#3B82F6", width=2)
        canvas.create_oval(798, 249, 804, 255, fill="#3B82F6", outline="white")
        canvas.create_text(816, 252, text="Balance Curve", font=("Segoe UI", 8), fill="#475569", anchor="w")

        # Plot Geometry
        plot_x0, plot_y0 = 100, 600
        plot_x1, plot_y1 = 900, 300

        # Dynamic max scale
        txns = account._transactions[-10:] if len(account._transactions) > 10 else account._transactions
        max_val = 10000.0
        for t in txns:
            if t.amount > max_val: max_val = t.amount
            if t.balance_after > max_val: max_val = t.balance_after
        max_val = (int(max_val // 5000) + 1) * 5000.0

        # Grid lines & Y scale
        for i in range(6):
            y_pos = plot_y0 - int((i / 5.0) * (plot_y0 - plot_y1))
            canvas.create_line(plot_x0, y_pos, plot_x1, y_pos, fill="#F1F5F9", width=1)
            val = (max_val / 5.0) * i
            canvas.create_text(plot_x0 - 10, y_pos, text=f"{int(val):,}", font=("Segoe UI", 8), fill="#94A3B8", anchor="e")

        # Axes
        canvas.create_line(plot_x0, plot_y1 - 10, plot_x0, plot_y0, fill="#94A3B8", width=2)
        canvas.create_line(plot_x0, plot_y0, plot_x1 + 10, plot_y0, fill="#94A3B8", width=2)

        if txns:
            step_x = (plot_x1 - plot_x0) // (len(txns) + 1)
            bar_w = max(18, step_x // 3)
            bal_points = []

            for idx, t in enumerate(txns):
                cx = plot_x0 + (idx + 1) * step_x
                bar_h = int((t.amount / max_val) * (plot_y0 - plot_y1))
                bar_h = max(3, bar_h)

                b_color = "#EF4444" if t.txn_type == "WITHDRAWAL" else "#10B981"
                canvas.create_rectangle(cx - bar_w // 2, plot_y0 - bar_h, cx + bar_w // 2, plot_y0,
                                        fill=b_color, outline="")

                # Amount text
                canvas.create_text(cx, plot_y0 - bar_h - 10, text=f"{int(t.amount):,}",
                                   font=("Segoe UI", 7), fill=b_color, anchor="center")
                # Txn ID
                canvas.create_text(cx, plot_y0 + 12, text=t.txn_id,
                                   font=("Segoe UI", 7), fill="#64748B", anchor="center")

                # Balance line coordinate
                bal_y = plot_y0 - int((t.balance_after / max_val) * (plot_y0 - plot_y1))
                bal_points.append((cx, bal_y))

            # Draw balance polyline
            if len(bal_points) > 1:
                flat_pts = [coord for pt in bal_points for coord in pt]
                canvas.create_line(*flat_pts, fill="#3B82F6", width=3, smooth=False)

            # Circular marker dots
            for px, py in bal_points:
                canvas.create_oval(px - 4, py - 4, px + 4, py + 4, fill="#3B82F6", outline="white", width=2)

        canvas.create_text(50, 670, text="Computer Graphics Module | Rendered via 2D Canvas Transformations",
                           font=("Segoe UI", 9, "italic"), fill="#94A3B8", anchor="w")


# ==============================================================================
# 7. Modern Tkinter GUI Front-End
# ==============================================================================

class BankingGUI:
    def __init__(self, root, bank: Bank):
        if tk is None:
            raise RuntimeError("Tkinter is not installed on this system.")
        self.root = root
        self.bank = bank
        self.root.title("Apex National Bank | OOP & Computer Graphics Mini-Project")
        self.root.geometry("820x640")
        self.root.configure(bg="#F1F5F9")
        self._build_ui()

    def _build_ui(self):
        # Header
        header = tk.Frame(self.root, bg="#0F172A", height=70)
        header.pack(fill="x")
        tk.Label(header, text="APEX NATIONAL BANK", font=("Segoe UI", 18, "bold"), fg="white", bg="#0F172A").pack(pady=(12, 2))
        tk.Label(header, text="OOP & Computer Graphics Mini-Project", font=("Segoe UI", 9), fg="#94A3B8", bg="#0F172A").pack()

        # Action Buttons Grid
        btn_frame = tk.Frame(self.root, bg="#F1F5F9", pady=15)
        btn_frame.pack(fill="x", padx=30)

        buttons = [
            ("a. Create an account", self.action_create_account, "#0284C7"),
            ("b. Deposit money", self.action_deposit, "#059669"),
            ("c. Withdraw money", self.action_withdraw, "#DC2626"),
            ("d. Honor daily withdrawal limit", self.action_daily_limit, "#D97706"),
            ("e. Check the balance", self.action_check_balance, "#4F46E5"),
            ("f. Display Account information.", self.action_display_info, "#0F172A"),
            ("g. Passbook Print (from to)", self.action_passbook, "#2563EB"),
            ("★ CG 2D Analytics", self.action_visualizer, "#7C3AED"),
        ]

        for i, (text, cmd, color) in enumerate(buttons):
            btn = tk.Button(btn_frame, text=text, font=("Segoe UI", 10, "bold"), fg="white", bg=color,
                            activebackground="#1E293B", activeforeground="white", width=22, height=2,
                            relief="flat", cursor="hand2", command=cmd)
            btn.grid(row=i // 4, column=i % 4, padx=6, pady=6)

        # Output Text Box Area
        out_frame = tk.Frame(self.root, bg="#F1F5F9", padx=30, pady=10)
        out_frame.pack(fill="both", expand=True)

        tk.Label(out_frame, text="System Console & Passbook Output Display:", font=("Segoe UI", 10, "bold"),
                 fg="#334155", bg="#F1F5F9").pack(anchor="w", pady=(0, 4))

        self.txt_out = tk.Text(out_frame, font=("Courier New", 9), bg="#0F172A", fg="#38BDF8",
                               insertbackground="white", relief="flat", padx=10, pady=10)
        self.txt_out.pack(fill="both", expand=True)

        self.log("Welcome to Apex National Bank System.\nReady for operations [a] through [g].\nSample accounts: SB1001 (PIN: 1234), CA2001 (PIN: 9999).\n")

    def log(self, text):
        self.txt_out.delete("1.0", tk.END)
        self.txt_out.insert(tk.END, text)

    def action_create_account(self):
        dialog = tk.Toplevel(self.root)
        dialog.title("a. Create An Account")
        dialog.geometry("400x420")
        dialog.configure(bg="#F8FAFC")

        fields = [
            ("Full Name:", "Vaishnavi Jare"),
            ("Phone Number:", "+91 9876543210"),
            ("Email Address:", "vaishnavi@sppu.edu"),
            ("Account Type (SAVINGS/CURRENT):", "SAVINGS"),
            ("4-Digit PIN:", "1234"),
            ("Initial Deposit (INR):", "5000"),
            ("Custom Daily Limit (0 for default):", "25000"),
        ]
        entries = {}
        for row, (lbl, default) in enumerate(fields):
            tk.Label(dialog, text=lbl, bg="#F8FAFC", font=("Segoe UI", 9, "bold")).pack(anchor="w", padx=25, pady=(6, 1))
            e = tk.Entry(dialog, font=("Segoe UI", 10))
            e.insert(0, default)
            e.pack(fill="x", padx=25)
            entries[lbl] = e

        def submit():
            try:
                name = entries["Full Name:"].get().strip()
                phone = entries["Phone Number:"].get().strip()
                email = entries["Email Address:"].get().strip()
                atype = entries["Account Type (SAVINGS/CURRENT):"].get().strip()
                pin = entries["4-Digit PIN:"].get().strip()
                dep = float(entries["Initial Deposit (INR):"].get())
                limit = float(entries["Custom Daily Limit (0 for default):"].get())

                acc_no = self.bank.create_account(name, phone, email, atype, pin, dep, limit)
                messagebox.showinfo("Success", f"Account created successfully!\nAccount Number: {acc_no}")
                self.log(f"SUCCESS: Created new {atype} Account {acc_no} for {name}.\nInitial Deposit: INR {dep:,.2f}")
                dialog.destroy()
            except Exception as ex:
                messagebox.showerror("Error", str(ex))

        tk.Button(dialog, text="Submit & Create Account", bg="#0284C7", fg="white", font=("Segoe UI", 10, "bold"),
                  relief="flat", cursor="hand2", command=submit).pack(pady=15, fill="x", padx=25)

    def action_deposit(self):
        dialog = tk.Toplevel(self.root)
        dialog.title("b. Deposit Money")
        dialog.geometry("350x260")
        dialog.configure(bg="#F8FAFC")

        tk.Label(dialog, text="Account Number:", bg="#F8FAFC", font=("Segoe UI", 9, "bold")).pack(anchor="w", padx=25, pady=(15, 2))
        e_acc = tk.Entry(dialog, font=("Segoe UI", 10)); e_acc.insert(0, "SB1001"); e_acc.pack(fill="x", padx=25)

        tk.Label(dialog, text="Deposit Amount (INR):", bg="#F8FAFC", font=("Segoe UI", 9, "bold")).pack(anchor="w", padx=25, pady=(10, 2))
        e_amt = tk.Entry(dialog, font=("Segoe UI", 10)); e_amt.insert(0, "2000"); e_amt.pack(fill="x", padx=25)

        tk.Label(dialog, text="Description (Optional):", bg="#F8FAFC", font=("Segoe UI", 9, "bold")).pack(anchor="w", padx=25, pady=(10, 2))
        e_desc = tk.Entry(dialog, font=("Segoe UI", 10)); e_desc.insert(0, "Online Deposit"); e_desc.pack(fill="x", padx=25)

        def submit():
            try:
                acc_no = e_acc.get().strip()
                amt = float(e_amt.get())
                desc = e_desc.get().strip()
                self.bank.deposit(acc_no, amt, desc)
                acc = self.bank.get_account(acc_no)
                messagebox.showinfo("Success", f"INR {amt:,.2f} deposited!\nNew Balance: INR {acc._balance:,.2f}")
                self.log(f"DEPOSIT CONFIRMED:\n  • Account : {acc_no}\n  • Amount  : INR {amt:,.2f}\n  • Balance : INR {acc._balance:,.2f}")
                dialog.destroy()
            except Exception as ex:
                messagebox.showerror("Error", str(ex))

        tk.Button(dialog, text="Confirm Deposit", bg="#059669", fg="white", font=("Segoe UI", 10, "bold"),
                  relief="flat", cursor="hand2", command=submit).pack(pady=15, fill="x", padx=25)

    def action_withdraw(self):
        dialog = tk.Toplevel(self.root)
        dialog.title("c. Withdraw Money (Honoring Daily Limit)")
        dialog.geometry("350x290")
        dialog.configure(bg="#F8FAFC")

        tk.Label(dialog, text="Account Number:", bg="#F8FAFC", font=("Segoe UI", 9, "bold")).pack(anchor="w", padx=25, pady=(12, 2))
        e_acc = tk.Entry(dialog, font=("Segoe UI", 10)); e_acc.insert(0, "SB1001"); e_acc.pack(fill="x", padx=25)

        tk.Label(dialog, text="4-Digit PIN:", bg="#F8FAFC", font=("Segoe UI", 9, "bold")).pack(anchor="w", padx=25, pady=(8, 2))
        e_pin = tk.Entry(dialog, font=("Segoe UI", 10), show="*"); e_pin.insert(0, "1234"); e_pin.pack(fill="x", padx=25)

        tk.Label(dialog, text="Withdrawal Amount (INR):", bg="#F8FAFC", font=("Segoe UI", 9, "bold")).pack(anchor="w", padx=25, pady=(8, 2))
        e_amt = tk.Entry(dialog, font=("Segoe UI", 10)); e_amt.insert(0, "3000"); e_amt.pack(fill="x", padx=25)

        def submit():
            try:
                acc_no = e_acc.get().strip()
                pin = e_pin.get().strip()
                amt = float(e_amt.get())
                self.bank.withdraw(acc_no, pin, amt, "ATM Cash Withdrawal")
                acc = self.bank.get_account(acc_no)
                messagebox.showinfo("Approved", f"Cash Dispensed: INR {amt:,.2f}\nUpdated Balance: INR {acc._balance:,.2f}")
                self.log(f"TRANSACTION APPROVED:\n  • Account          : {acc_no}\n  • Cash Dispensed   : INR {amt:,.2f}\n  • Updated Balance  : INR {acc._balance:,.2f}\n  • Remaining Limit  : INR {acc.get_remaining_daily_limit():,.2f}")
                dialog.destroy()
            except DailyLimitExceededException as de:
                messagebox.showerror("Limit Exceeded", str(de))
                self.log(f"[TRANSACTION DECLINED - DAILY LIMIT]\n{str(de)}")
            except Exception as ex:
                messagebox.showerror("Error", str(ex))
                self.log(f"[TRANSACTION DECLINED]\n{str(ex)}")

        tk.Button(dialog, text="Authorize & Withdraw Cash", bg="#DC2626", fg="white", font=("Segoe UI", 10, "bold"),
                  relief="flat", cursor="hand2", command=submit).pack(pady=15, fill="x", padx=25)

    def action_daily_limit(self):
        dialog = tk.Toplevel(self.root)
        dialog.title("d. Daily Withdrawal Limit Status")
        dialog.geometry("350x230")
        dialog.configure(bg="#F8FAFC")

        tk.Label(dialog, text="Account Number:", bg="#F8FAFC", font=("Segoe UI", 9, "bold")).pack(anchor="w", padx=25, pady=(15, 2))
        e_acc = tk.Entry(dialog, font=("Segoe UI", 10)); e_acc.insert(0, "SB1001"); e_acc.pack(fill="x", padx=25)

        tk.Label(dialog, text="4-Digit PIN:", bg="#F8FAFC", font=("Segoe UI", 9, "bold")).pack(anchor="w", padx=25, pady=(10, 2))
        e_pin = tk.Entry(dialog, font=("Segoe UI", 10), show="*"); e_pin.insert(0, "1234"); e_pin.pack(fill="x", padx=25)

        def submit():
            try:
                acc_no = e_acc.get().strip()
                pin = e_pin.get().strip()
                info = self.bank.honor_daily_limit(acc_no, pin)
                acc = self.bank.get_account(acc_no)
                res = (
                    f"==============================================================\n"
                    f"     DAILY WITHDRAWAL LIMIT STATUS: {acc_no} ({acc._holder_name})\n"
                    f"==============================================================\n"
                    f"  • Configured Daily Limit : INR {info['daily_limit']:,.2f}\n"
                    f"  • Withdrawn Today        : INR {info['withdrawn_today']:,.2f}\n"
                    f"  • Remaining Allowance    : INR {info['remaining_limit']:,.2f}\n"
                    f"  • Last Withdrawal Date   : {info['last_date']}\n"
                    f"==============================================================\n"
                )
                self.log(res)
                dialog.destroy()
            except Exception as ex:
                messagebox.showerror("Error", str(ex))

        tk.Button(dialog, text="Check Daily Limit Status", bg="#D97706", fg="white", font=("Segoe UI", 10, "bold"),
                  relief="flat", cursor="hand2", command=submit).pack(pady=15, fill="x", padx=25)

    def action_check_balance(self):
        dialog = tk.Toplevel(self.root)
        dialog.title("e. Check the Balance")
        dialog.geometry("350x230")
        dialog.configure(bg="#F8FAFC")

        tk.Label(dialog, text="Account Number:", bg="#F8FAFC", font=("Segoe UI", 9, "bold")).pack(anchor="w", padx=25, pady=(15, 2))
        e_acc = tk.Entry(dialog, font=("Segoe UI", 10)); e_acc.insert(0, "SB1001"); e_acc.pack(fill="x", padx=25)

        tk.Label(dialog, text="4-Digit PIN:", bg="#F8FAFC", font=("Segoe UI", 9, "bold")).pack(anchor="w", padx=25, pady=(10, 2))
        e_pin = tk.Entry(dialog, font=("Segoe UI", 10), show="*"); e_pin.insert(0, "1234"); e_pin.pack(fill="x", padx=25)

        def submit():
            try:
                acc_no = e_acc.get().strip()
                pin = e_pin.get().strip()
                bal = self.bank.check_balance(acc_no, pin)
                acc = self.bank.get_account(acc_no)
                res = (
                    f"BALANCE ENQUIRY REPORT\n"
                    f"  • Account Number  : {acc_no}\n"
                    f"  • Account Holder  : {acc._holder_name}\n"
                    f"  • Account Type    : {acc.get_account_type()}\n"
                    f"  • Current Balance : INR {bal:,.2f}\n"
                    f"  • Minimum Balance : INR {acc.get_minimum_balance():,.2f}\n"
                    f"  • Today's Rem. Limit: INR {acc.get_remaining_daily_limit():,.2f}\n"
                )
                self.log(res)
                dialog.destroy()
            except Exception as ex:
                messagebox.showerror("Error", str(ex))

        tk.Button(dialog, text="Inquire Balance", bg="#4F46E5", fg="white", font=("Segoe UI", 10, "bold"),
                  relief="flat", cursor="hand2", command=submit).pack(pady=15, fill="x", padx=25)

    def action_display_info(self):
        dialog = tk.Toplevel(self.root)
        dialog.title("f. Display Account Information")
        dialog.geometry("350x180")
        dialog.configure(bg="#F8FAFC")

        tk.Label(dialog, text="Account Number:", bg="#F8FAFC", font=("Segoe UI", 9, "bold")).pack(anchor="w", padx=25, pady=(18, 2))
        e_acc = tk.Entry(dialog, font=("Segoe UI", 10)); e_acc.insert(0, "SB1001"); e_acc.pack(fill="x", padx=25)

        def submit():
            try:
                acc_no = e_acc.get().strip()
                acc = self.bank.get_account(acc_no)
                self.log(acc.display_account_info())
                dialog.destroy()
            except Exception as ex:
                messagebox.showerror("Error", str(ex))

        tk.Button(dialog, text="Fetch Account Details", bg="#0F172A", fg="white", font=("Segoe UI", 10, "bold"),
                  relief="flat", cursor="hand2", command=submit).pack(pady=15, fill="x", padx=25)

    def action_passbook(self):
        dialog = tk.Toplevel(self.root)
        dialog.title("g. Passbook Print (From Date to To Date)")
        dialog.geometry("400x320")
        dialog.configure(bg="#F8FAFC")

        tk.Label(dialog, text="Account Number:", bg="#F8FAFC", font=("Segoe UI", 9, "bold")).pack(anchor="w", padx=25, pady=(10, 2))
        e_acc = tk.Entry(dialog, font=("Segoe UI", 10)); e_acc.insert(0, "SB1001"); e_acc.pack(fill="x", padx=25)

        tk.Label(dialog, text="4-Digit PIN:", bg="#F8FAFC", font=("Segoe UI", 9, "bold")).pack(anchor="w", padx=25, pady=(6, 2))
        e_pin = tk.Entry(dialog, font=("Segoe UI", 10), show="*"); e_pin.insert(0, "1234"); e_pin.pack(fill="x", padx=25)

        tk.Label(dialog, text="From Date (YYYY-MM-DD):", bg="#F8FAFC", font=("Segoe UI", 9, "bold")).pack(anchor="w", padx=25, pady=(6, 2))
        e_from = tk.Entry(dialog, font=("Segoe UI", 10)); e_from.insert(0, "2026-01-01"); e_from.pack(fill="x", padx=25)

        tk.Label(dialog, text="To Date (YYYY-MM-DD):", bg="#F8FAFC", font=("Segoe UI", 9, "bold")).pack(anchor="w", padx=25, pady=(6, 2))
        e_to = tk.Entry(dialog, font=("Segoe UI", 10)); e_to.insert(0, datetime.date.today().strftime("%Y-%m-%d")); e_to.pack(fill="x", padx=25)

        def submit():
            try:
                acc_no = e_acc.get().strip()
                pin = e_pin.get().strip()
                from_d = datetime.date.fromisoformat(e_from.get().strip())
                to_d = datetime.date.fromisoformat(e_to.get().strip())

                acc = self.bank.get_account(acc_no)
                if not acc.authenticate(pin):
                    raise AuthenticationException()

                pb_text = acc.generate_passbook_text(from_d, to_d)
                self.log(pb_text)

                # Export to disk
                os.makedirs("data", exist_ok=True)
                with open(f"data/passbook_{acc_no}.txt", "w", encoding="utf-8") as f:
                    f.write(pb_text)

                dialog.destroy()
            except Exception as ex:
                messagebox.showerror("Error", str(ex))

        tk.Button(dialog, text="Print Passbook Statement", bg="#2563EB", fg="white", font=("Segoe UI", 10, "bold"),
                  relief="flat", cursor="hand2", command=submit).pack(pady=15, fill="x", padx=25)

    def action_visualizer(self):
        dialog = tk.Toplevel(self.root)
        dialog.title("2D Computer Graphics Analytics")
        dialog.geometry("350x180")
        dialog.configure(bg="#F8FAFC")

        tk.Label(dialog, text="Enter Account Number to Visualize:", bg="#F8FAFC", font=("Segoe UI", 9, "bold")).pack(anchor="w", padx=25, pady=(18, 2))
        e_acc = tk.Entry(dialog, font=("Segoe UI", 10)); e_acc.insert(0, "SB1001"); e_acc.pack(fill="x", padx=25)

        def submit():
            try:
                acc_no = e_acc.get().strip()
                acc = self.bank.get_account(acc_no)
                CGVisualizer.show_dashboard(acc)
                dialog.destroy()
            except Exception as ex:
                messagebox.showerror("Error", str(ex))

        tk.Button(dialog, text="Render 2D Graphics Dashboard", bg="#7C3AED", fg="white", font=("Segoe UI", 10, "bold"),
                  relief="flat", cursor="hand2", command=submit).pack(pady=15, fill="x", padx=25)


# ==============================================================================
# 8. Interactive CLI (Command Line Interface)
# ==============================================================================

def run_cli(bank: Bank):
    while True:
        print("\n" + "=" * 80)
        print("Banking system having the following operations:")
        print("  a. Create an account")
        print("  b. Deposit money")
        print("  c. Withdraw money")
        print("  d. Honor daily withdrawal limit")
        print("  e. Check the balance")
        print("  f. Display Account information.")
        print("  g. Passbook Print (from to)")
        print("--------------------------------------------------------------------------------")
        print("  h. Computer Graphics: Visual Analytics Dashboard (Bar Chart & Balance Trend)")
        print("  i. View All Accounts Directory")
        print("  q. Exit")
        print("=" * 80)

        raw_choice = input("Select an option (a-g, or q to exit): ").strip().lower()
        if not raw_choice:
            continue

        if raw_choice in ("a", "1"):
            print("\n--- [a] CREATE AN ACCOUNT ---")
            name = input("Account Holder Full Name : ").strip()
            phone = input("Contact Phone Number     : ").strip()
            email = input("Email Address            : ").strip()
            print("1. Savings (Min Bal: INR 1,000 | Daily Limit: INR 25,000)")
            print("2. Current (Min Bal: INR 5,000 | Daily Limit: INR 100,000)")
            tc = input("Choice (1 or 2): ").strip()
            atype = "CURRENT" if tc == "2" else "SAVINGS"
            pin = input("Set 4-Digit PIN          : ").strip()
            init_dep = float(input("Initial Deposit Amount   : INR "))
            try:
                acc_no = bank.create_account(name, phone, email, atype, pin, init_dep)
                print(f"[SUCCESS] Account created! Account Number: {acc_no}")
            except Exception as e:
                print(f"[ERROR] {e}")

        elif raw_choice in ("b", "2"):
            print("\n--- [b] DEPOSIT MONEY ---")
            acc_no = input("Account Number : ").strip()
            amt = float(input("Deposit Amount : INR "))
            desc = input("Remarks        : ").strip() or "Cash Deposit"
            try:
                bank.deposit(acc_no, amt, desc)
                acc = bank.get_account(acc_no)
                print(f"[SUCCESS] Deposited INR {amt:,.2f}. Current Balance: INR {acc._balance:,.2f}")
            except Exception as e:
                print(f"[ERROR] {e}")

        elif raw_choice in ("c", "3"):
            print("\n--- [c] WITHDRAW MONEY ---")
            acc_no = input("Account Number : ").strip()
            pin = input("4-Digit PIN    : ").strip()
            amt = float(input("Amount to Withdraw : INR "))
            try:
                bank.withdraw(acc_no, pin, amt, "Cash Withdrawal")
                acc = bank.get_account(acc_no)
                print(f"[TRANSACTION APPROVED] Dispensed: INR {amt:,.2f} | Remaining Bal: INR {acc._balance:,.2f}")
            except Exception as e:
                print(f"[DECLINED] {e}")

        elif raw_choice in ("d", "4"):
            print("\n--- [d] HONOR DAILY WITHDRAWAL LIMIT ---")
            acc_no = input("Account Number : ").strip()
            pin = input("4-Digit PIN    : ").strip()
            try:
                status = bank.honor_daily_limit(acc_no, pin)
                print(f"Daily Limit Allowed : INR {status['daily_limit']:,.2f}")
                print(f"Withdrawn Today     : INR {status['withdrawn_today']:,.2f}")
                print(f"Remaining for Today : INR {status['remaining_limit']:,.2f}")
            except Exception as e:
                print(f"[ERROR] {e}")

        elif raw_choice in ("e", "5"):
            print("\n--- [e] CHECK THE BALANCE ---")
            acc_no = input("Account Number : ").strip()
            pin = input("4-Digit PIN    : ").strip()
            try:
                bal = bank.check_balance(acc_no, pin)
                print(f"[BALANCE] INR {bal:,.2f}")
            except Exception as e:
                print(f"[ERROR] {e}")

        elif raw_choice in ("f", "6"):
            print("\n--- [f] DISPLAY ACCOUNT INFORMATION ---")
            acc_no = input("Account Number : ").strip()
            try:
                acc = bank.get_account(acc_no)
                print(acc.display_account_info())
            except Exception as e:
                print(f"[ERROR] {e}")

        elif raw_choice in ("g", "7"):
            print("\n--- [g] PASSBOOK PRINT (FROM TO) ---")
            acc_no = input("Account Number : ").strip()
            pin = input("4-Digit PIN    : ").strip()
            try:
                acc = bank.get_account(acc_no)
                if not acc.authenticate(pin):
                    raise AuthenticationException()
                from_s = input("From Date (YYYY-MM-DD, press Enter for all)  : ").strip() or "1970-01-01"
                to_s   = input("To Date   (YYYY-MM-DD, press Enter for today): ").strip() or datetime.date.today().isoformat()
                try:
                    from_d = datetime.date.fromisoformat(from_s)
                except ValueError:
                    from_d = datetime.date(1970, 1, 1)
                try:
                    to_d = datetime.date.fromisoformat(to_s)
                except ValueError:
                    to_d = datetime.date.today()
                print(acc.generate_passbook_text(from_d, to_d))
            except Exception as e:
                print(f"[ERROR] {e}")

        elif raw_choice in ("h", "8"):
            print("\n--- COMPUTER GRAPHICS VISUAL ANALYTICS ---")
            acc_no = input("Account Number to visualize : ").strip()
            try:
                acc = bank.get_account(acc_no)
                if tk is None:
                    print("\n[!] Tkinter is not installed on this system.")
                    print("    The 2D graphical visualizer is unavailable in headless/CLI environments.")
                    print("    Please use the web app (web_app.py) or install python3-tk.")
                else:
                    CGVisualizer.show_dashboard(acc)
            except Exception as e:
                print(f"[ERROR] {e}")

        elif raw_choice in ("i", "9"):
            print("\n--- ALL REGISTERED ACCOUNTS ---")
            for acc in bank.accounts.values():
                print(f"{acc._account_number:8} | {acc.get_account_type():8} | {acc._holder_name:24} | Bal: INR {acc._balance:10.2f}")

        elif raw_choice in ("q", "0", "exit"):
            print("\nExiting. Thank you for using Apex Banking System!")
            break

        else:
            print(f"[!] Invalid option '{raw_choice}'. Please enter a letter from 'a' to 'g', or 'q' to exit.")


# ==============================================================================
# Main Entry Point
# ==============================================================================

if __name__ == "__main__":
    bank_system = Bank()

    if "--cli" in sys.argv or tk is None:
        run_cli(bank_system)
    else:
        try:
            root = tk.Tk()
            app = BankingGUI(root, bank_system)
            root.mainloop()
        except Exception as e:
            # Fallback to CLI if headless, display error, or GUI fails
            print(f"[NOTE] GUI could not start ({e}). Launching CLI mode instead...\n")
            run_cli(bank_system)
