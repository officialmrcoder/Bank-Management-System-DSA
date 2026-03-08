Bank Management System in C++
Objective

Design and implement a secure banking transaction system using C++.
The system uses OOP, pointers, vectors, arrays, file handling, and bitwise operators to manage accounts and transactions efficiently.

Concepts Covered

Object-Oriented Programming (Inheritance, Polymorphism, Encapsulation)

Pointers and Dynamic Memory

File Handling (fstream)

Arrays

Vectors

Bitwise Operators

System Design Thinking

Functional Requirements
1) Account Class Hierarchy

Abstract base class: Account

Derived classes: SavingsAccount, CurrentAccount

Supports runtime polymorphism

Handles deposit(), withdraw(), saveToFile(), loadFromFile()

class Account {
protected:
    int accountId;
    string name;
    double balance;
    unsigned int permissions;
    vector<double> transactions;

public:
    virtual void deposit(double amount) = 0;
    virtual void withdraw(double amount) = 0;
    virtual void saveToFile() = 0;
    virtual void loadFromFile() = 0;
    virtual ~Account() {}
};
2) Bitwise Permission System

Permissions stored as bit flags in unsigned int

Example flags:

1 → Can Withdraw

2 → Can Deposit

4 → Can Transfer

8 → VIP Account

Use bitwise OR to assign permissions and bitwise AND to check permissions

if (permissions & 1) { /* can withdraw */ }
3) Pointer Usage

All accounts created dynamically

Account* acc = new SavingsAccount(...);

Stored in: vector<Account*> accounts;

Proper destructors implemented to avoid memory leaks

4) Transaction Management

Each account stores transaction history using vector<double>

Positive values → deposits

Negative values → withdrawals

5) File Handling

Save and load all accounts to/from files

Example file format:

ACCOUNT Savings
1001 Ali 5000 7
TRANSACTIONS
1000
-500
300

ACCOUNT Current
1002 Sara 8000 3
TRANSACTIONS
2000
-1000

Must save: account type, permissions, transaction history

Correctly reconstruct objects when loading

6) Monthly Summary Using Arrays

Use array: double monthlyTotals[12];

Calculate total deposits, withdrawals, and net monthly balance change

7) Transaction Compression (Advanced)

Compress transaction type and amount into a single unsigned int

Structure: [ 4 bits type ][ 28 bits amount ]

Transaction types:

1 → Deposit

2 → Withdrawal

3 → Transfer

Use bit shifting, masking, AND, OR for encoding/decoding

8) Sample Interaction

Create Account

Deposit

Withdraw

Show Account

Save to File

Load from File

Exit

Constraints

Do NOT use STL map or set

Must use vector<Account*> and dynamic memory

Must use bitwise operators explicitly

Proper destructor implementation required

Bonus Features

Account transfer between accounts

Overdraft limit

Interest calculation

File encryption using simple XOR cipher

Admin mode with full permissions

Evaluation Criteria

Correct use of OOP and polymorphism

Proper dynamic memory handling

Correct bitwise logic

File persistence

Clean modular design

No memory leaks

Author

Anas Ali
