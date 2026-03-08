#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include<windows.h>
#include<conio.h>
#include<iomanip>
using namespace std;

// permission flags using bits
const unsigned int CAN_WITHDRAW = 1;
const unsigned int CAN_DEPOSIT = 2;
const unsigned int CAN_TRANSFER = 4;
const unsigned int IS_VIP = 8;

// for transaction compression
const int DEPOSIT_TYPE = 1;
const int WITHDRAW_TYPE = 2;
const int TRANSFER_TYPE = 3;

// file name
const char DATA_FILE[] = "bankdata.txt";

void gotoxy(int x, int y) {
    COORD c;
    c.X = x;
    c.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

// abstract base class
class Account {
protected:
    int accId;
    string accName;
    double balance;
    unsigned int perm;  // permissions stored as bits
    vector<double> history;  // transaction history
    vector<unsigned int> compressed; // compressed transactions
    string type;
    
public:
    Account(int id, string n, double bal, unsigned int p, string t) {
        accId = id;
        accName = n;
        balance = bal;
        perm = p;
        type = t;
    }
    
    virtual ~Account() {
        // cleanup
        history.clear();
        compressed.clear();
    }
    
    // pure virtual functions
    virtual void deposit(double amt) = 0;
    virtual void withdraw(double amt) = 0;
    virtual void saveData(ofstream &f) = 0;
    virtual void loadData(ifstream &f) = 0;
    virtual void show() = 0;
    virtual void addInterest() = 0;
    virtual string getAccType() = 0;
    
    // check permission using bitwise AND
    bool checkPerm(unsigned int p) {
        if((perm & p) != 0) return true;
        return false;
    }
    
    // add permission using OR
    void addPerm(unsigned int p) {
        perm = perm | p;
    }
    
    // remove permission
    void removePerm(unsigned int p) {
        perm = perm & (~p);
    }
    
    // compress transaction: 4 bits type, 28 bits amount
    unsigned int compress(int t, double amt) {
        unsigned int a = (unsigned int)(amt * 100); // store as cents
        unsigned int result = (t << 28) | (a & 0x0FFFFFFF);
        return result;
    }
    
    // decompress
    void decompress(unsigned int data, int &t, double &amt) {
        t = (data >> 28) & 0x0F;
        unsigned int a = data & 0x0FFFFFFF;
        amt = a / 100.0;
    }
    
    void record(double amt, int t) {
        history.push_back(amt);
        unsigned int c = compress(t, abs(amt));
        compressed.push_back(c);
    }
    
    // getters
    int getId() { return accId; }
    string getName() { return accName; }
    double getBal() { return balance; }
    string getType() { return type; }
    
    // setter for balance
    void setBal(double b) { balance = b; }
    
    // transfer to another account
    bool transfer(Account *other, double amt) {
        if(!checkPerm(CAN_TRANSFER)) {
            cout << "No transfer permission!\n";
            return false;
        }
        if(!checkPerm(CAN_WITHDRAW)) {
            cout << "No withdraw permission!\n";
            return false;
        }
        
        if(amt > balance) {
            cout << "Not enough money!\n";
            return false;
        }
        
        balance -= amt;
        record(-amt, TRANSFER_TYPE);
        
        other->balance += amt;
        other->record(amt, TRANSFER_TYPE);
        
        cout << "Transferred " << amt << " to account " << other->getId() << "\n";
        return true;
    }
    
    void showHistory() {
        cout << "\n--- Transaction History ---\n";
        if(history.size() == 0) {
            cout << "No transactions\n";
            return;
        }
        
        for(int i=0; i<history.size(); i++) {
            cout << i+1 << ". ";
            if(history[i] > 0)
                cout << "Deposit: +" << history[i];
            else
                cout << "Withdraw: " << history[i];
            cout << "\n";
        }
    }
    
    void showPerm() {
        cout << "Permissions: ";
        if(checkPerm(CAN_DEPOSIT)) cout << "DEPOSIT ";
        if(checkPerm(CAN_WITHDRAW)) cout << "WITHDRAW ";
        if(checkPerm(CAN_TRANSFER)) cout << "TRANSFER ";
        if(checkPerm(IS_VIP)) cout << "VIP";
        cout << "\n";
    }
    
    // save common stuff
    void saveCommon(ofstream &f) {
        f << accId << "\n";
        f << accName << "\n";
        f << balance << "\n";
        f << perm << "\n";
        f << type << "\n";
        f << history.size() << "\n";
        
        for(int i=0; i<history.size(); i++) {
            f << history[i] << " " << compressed[i] << "\n";
        }
    }
    
    // load common stuff
    void loadCommon(ifstream &f) {
        f >> accId;
        f.ignore();
        getline(f, accName);
        f >> balance;
        f >> perm;
        f.ignore();
        getline(f, type);
        
        int n;
        f >> n;
        
        history.clear();
        compressed.clear();
        
        for(int i=0; i<n; i++) {
            double h;
            unsigned int c;
            f >> h >> c;
            history.push_back(h);
            compressed.push_back(c);
        }
    }
};

// savings account
class Savings : public Account {
    double interestRate;
public:
    Savings(int id, string n, double bal, unsigned int p) 
        : Account(id, n, bal, p, "Savings") {
        interestRate = 0.04; // 4%
    }
    
    void deposit(double amt) {
        if(!checkPerm(CAN_DEPOSIT)) {
            cout << "Deposit not allowed!\n";
            return;
        }
        if(amt <= 0) {
            cout << "Invalid amount!\n";
            return;
        }
        
        balance += amt;
        record(amt, DEPOSIT_TYPE);
        cout << "Deposited " << amt << ". New balance: " << balance << "\n";
    }
    
    void withdraw(double amt) {
        if(!checkPerm(CAN_WITHDRAW)) {
            cout << "Withdraw not allowed!\n";
            return;
        }
        if(amt <= 0) {
            cout << "Invalid amount!\n";
            return;
        }
        if(amt > balance) {
            cout << "Insufficient funds!\n";
            return;
        }
        
        balance -= amt;
        record(-amt, WITHDRAW_TYPE);
        cout << "Withdrawn " << amt << ". New balance: " << balance << "\n";
    }
    
    void addInterest() {
        double interest = balance * interestRate / 12;
        balance += interest;
        record(interest, DEPOSIT_TYPE);
        cout << "Interest added: " << interest << "\n";
    }
    
    void saveData(ofstream &f) {
        f << "ACCOUNT Savings\n";
        saveCommon(f);
        f << interestRate << "\n";
        f << "END\n";
    }
    
    void loadData(ifstream &f) {
        loadCommon(f);
        f >> interestRate;
        string end;
        f >> end;
    }
    
    void show() {
        cout << "\n======== SAVINGS ACCOUNT ========\n";
        cout << "ID: " << accId << "\n";
        cout << "Name: " << accName << "\n";
        cout << "Balance: " << fixed << setprecision(2) << balance << "\n";
        cout << "Interest: " << interestRate*100 << "%\n";
        showPerm();
        showHistory();
        cout << "=================================\n";
    }
    
    string getAccType() { return "Savings"; }
};

// current account
class Current : public Account {
    double overdraft;
public:
    Current(int id, string n, double bal, unsigned int p, double od=1000) 
        : Account(id, n, bal, p, "Current") {
        overdraft = od;
    }
    
    void deposit(double amt) {
        if(!checkPerm(CAN_DEPOSIT)) {
            cout << "Deposit not allowed!\n";
            return;
        }
        if(amt <= 0) {
            cout << "Invalid amount!\n";
            return;
        }
        
        balance += amt;
        record(amt, DEPOSIT_TYPE);
        cout << "Deposited " << amt << ". New balance: " << balance << "\n";
    }
    
    void withdraw(double amt) {
        if(!checkPerm(CAN_WITHDRAW)) {
            cout << "Withdraw not allowed!\n";
            return;
        }
        if(amt <= 0) {
            cout << "Invalid amount!\n";
            return;
        }
        
        double maxWithdraw = balance + overdraft;
        if(amt > maxWithdraw) {
            cout << "Exceeds overdraft limit!\n";
            return;
        }
        
        balance -= amt;
        record(-amt, WITHDRAW_TYPE);
        cout << "Withdrawn " << amt << "\n";
        if(balance < 0)
            cout << "WARNING: Using overdraft. Balance: " << balance << "\n";
        else
            cout << "New balance: " << balance << "\n";
    }
    
    void addInterest() {
        cout << "Current account has no interest\n";
    }
    
    void saveData(ofstream &f) {
        f << "ACCOUNT Current\n";
        saveCommon(f);
        f << overdraft << "\n";
        f << "END\n";
    }
    
    void loadData(ifstream &f) {
        loadCommon(f);
        f >> overdraft;
        string end;
        f >> end;
    }
    
    void show() {
        cout << "\n======== CURRENT ACCOUNT ========\n";
        cout << "ID: " << accId << "\n";
        cout << "Name: " << accName << "\n";
        cout << "Balance: " << fixed << setprecision(2) << balance << "\n";
        cout << "Overdraft: " << overdraft << "\n";
        cout << "Available: " << (balance + overdraft) << "\n";
        showPerm();
        showHistory();
        cout << "=================================\n";
    }
    
    string getAccType() { return "Current"; }
};

// main bank class
class Bank {
    vector<Account*> accounts;
    double monthly[12]; // array for monthly summary
    
public:
    Bank() {
        for(int i=0; i<12; i++) monthly[i] = 0;
    }
    
    ~Bank() {
        // delete all accounts to prevent memory leak
        for(int i=0; i<accounts.size(); i++) {
            delete accounts[i];
        }
        accounts.clear();
    }
    
    Account* find(int id) {
        for(int i=0; i<accounts.size(); i++) {
            if(accounts[i]->getId() == id)
                return accounts[i];
        }
        return NULL;
    }
    
    void create() {
        system("cls");
        gotoxy(30, 2);
        cout << "===== CREATE ACCOUNT =====";
        
        int id, t;
        string name;
        double bal;
        unsigned int p = 0;
        char ch;
        
        gotoxy(30, 4);
        cout << "Enter ID: ";
        cin >> id;
        
        if(find(id) != NULL) {
            gotoxy(30, 6);
            cout << "ID already exists!";
            return;
        }
        
        gotoxy(30, 5);
        cout << "Enter Name: ";
        cin.ignore();
        getline(cin, name);
        
        gotoxy(30, 6);
        cout << "Enter Balance: ";
        cin >> bal;
        
        gotoxy(30, 7);
        cout << "Type (1-Savings, 2-Current): ";
        cin >> t;
        
        // set permissions
        gotoxy(30, 9);
        cout << "Set permissions (y/n):";
        
        gotoxy(30, 10);
        cout << "Deposit? ";
        cin >> ch;
        if(ch=='y' || ch=='Y') p = p | CAN_DEPOSIT;
        
        gotoxy(30, 11);
        cout << "Withdraw? ";
        cin >> ch;
        if(ch=='y' || ch=='Y') p = p | CAN_WITHDRAW;
        
        gotoxy(30, 12);
        cout << "Transfer? ";
        cin >> ch;
        if(ch=='y' || ch=='Y') p = p | CAN_TRANSFER;
        
        gotoxy(30, 13);
        cout << "VIP? ";
        cin >> ch;
        if(ch=='y' || ch=='Y') p = p | IS_VIP;
        
        Account *a = NULL;
        
        if(t == 1) {
            a = new Savings(id, name, bal, p);
        } else if(t == 2) {
            double od;
            gotoxy(30, 14);
            cout << "Overdraft limit: ";
            cin >> od;
            a = new Current(id, name, bal, p, od);
        } else {
            gotoxy(30, 16);
            cout << "Invalid type!";
            return;
        }
        
        accounts.push_back(a);
        gotoxy(30, 17);
        cout << "Account created!";
    }
    
    void doDeposit() {
        system("cls");
        gotoxy(30, 2);
        cout << "===== DEPOSIT =====";
        
        int id;
        double amt;
        
        gotoxy(30, 4);
        cout << "Account ID: ";
        cin >> id;
        
        Account *a = find(id);
        if(a == NULL) {
            gotoxy(30, 6);
            cout << "Not found!";
            return;
        }
        
        gotoxy(30, 5);
        cout << "Amount: ";
        cin >> amt;
        
        a->deposit(amt);
        
        // update monthly (using january for demo)
        if(amt > 0) monthly[0] += amt;
    }
    
    void doWithdraw() {
        system("cls");
        gotoxy(30, 2);
        cout << "===== WITHDRAW =====";
        
        int id;
        double amt;
        
        gotoxy(30, 4);
        cout << "Account ID: ";
        cin >> id;
        
        Account *a = find(id);
        if(a == NULL) {
            gotoxy(30, 6);
            cout << "Not found!";
            return;
        }
        
        gotoxy(30, 5);
        cout << "Amount: ";
        cin >> amt;
        
        a->withdraw(amt);
        
        if(amt > 0) monthly[0] -= amt;
    }
    
    void doTransfer() {
        system("cls");
        gotoxy(30, 2);
        cout << "===== TRANSFER =====";
        
        int from, to;
        double amt;
        
        gotoxy(30, 4);
        cout << "From ID: ";
        cin >> from;
        
        gotoxy(30, 5);
        cout << "To ID: ";
        cin >> to;
        
        gotoxy(30, 6);
        cout << "Amount: ";
        cin >> amt;
        
        Account *a1 = find(from);
        Account *a2 = find(to);
        
        if(a1 == NULL || a2 == NULL) {
            gotoxy(30, 8);
            cout << "One or both accounts not found!";
            return;
        }
        
        a1->transfer(a2, amt);
    }
    
    void showAccount() {
        system("cls");
        gotoxy(30, 2);
        cout << "===== ACCOUNT DETAILS =====";
        
        int id;
        gotoxy(30, 4);
        cout << "Enter ID: ";
        cin >> id;
        
        Account *a = find(id);
        if(a == NULL) {
            gotoxy(30, 6);
            cout << "Not found!";
            return;
        }
        
        a->show();
    }
    
    void listAll() {
        system("cls");
        gotoxy(30, 2);
        cout << "===== ALL ACCOUNTS =====";
        
        if(accounts.size() == 0) {
            gotoxy(30, 4);
            cout << "No accounts";
            return;
        }
        
        int row = 4;
        for(int i=0; i<accounts.size(); i++) {
            gotoxy(30, row++);
            cout << accounts[i]->getId() << " - " 
                 << accounts[i]->getName() << " - "
                 << accounts[i]->getAccType() << " - $"
                 << accounts[i]->getBal();
        }
    }
    
    void delAccount() {
        system("cls");
        gotoxy(30, 2);
        cout << "===== DELETE ACCOUNT =====";
        
        int id;
        gotoxy(30, 4);
        cout << "Enter ID to delete: ";
        cin >> id;
        
        for(int i=0; i<accounts.size(); i++) {
            if(accounts[i]->getId() == id) {
                delete accounts[i]; // free memory
                accounts.erase(accounts.begin() + i);
                gotoxy(30, 6);
                cout << "Deleted!";
                return;
            }
        }
        
        gotoxy(30, 6);
        cout << "Not found!";
    }
    
    void saveAll() {
        ofstream f(DATA_FILE);
        if(!f) {
            cout << "Error opening file!\n";
            return;
        }
        
        f << accounts.size() << "\n";
        
        for(int i=0; i<accounts.size(); i++) {
            accounts[i]->saveData(f);
        }
        
        f.close();
        
        // save monthly data
        ofstream mf("monthly.txt");
        for(int i=0; i<12; i++) {
            mf << monthly[i] << "\n";
        }
        mf.close();
        
        cout << "Saved!\n";
    }
    
    void loadAll() {
        // clear existing
        for(int i=0; i<accounts.size(); i++) {
            delete accounts[i];
        }
        accounts.clear();
        
        ifstream f(DATA_FILE);
        if(!f) {
            cout << "No data file found\n";
            return;
        }
        
        int n;
        f >> n;
        f.ignore();
        
        for(int i=0; i<n; i++) {
            string marker, typ;
            f >> marker >> typ;
            
            Account *a = NULL;
            if(typ == "Savings") {
                a = new Savings(0, "", 0, 0);
            } else if(typ == "Current") {
                a = new Current(0, "", 0, 0);
            }
            
            if(a != NULL) {
                a->loadData(f);
                accounts.push_back(a);
            }
        }
        
        f.close();
        
        // load monthly
        ifstream mf("monthly.txt");
        if(mf) {
            for(int i=0; i<12; i++) {
                mf >> monthly[i];
            }
            mf.close();
        }
        
        cout << "Loaded!\n";
    }
    
    void showMonthly() {
        system("cls");
        gotoxy(30, 2);
        cout << "===== MONTHLY SUMMARY =====";
        
        string months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        
        int row = 4;
        for(int i=0; i<12; i++) {
            gotoxy(30, row++);
            cout << months[i] << ": ";
            if(monthly[i] >= 0)
                cout << "+" << monthly[i];
            else
                cout << monthly[i];
        }
        
        // total
        double total = 0;
        for(int i=0; i<12; i++) total += monthly[i];
        
        gotoxy(30, row+1);
        cout << "Total: " << total;
    }
    
    void admin() {
        system("cls");
        string pass;
        gotoxy(30, 2);
        cout << "===== ADMIN MODE =====";
        gotoxy(30, 4);
        cout << "Password: ";
        cin >> pass;
        
        if(pass != "admin") {
            gotoxy(30, 6);
            cout << "Wrong password!";
            return;
        }
        
        gotoxy(30, 6);
        cout << "Access granted";
        gotoxy(30, 8);
        cout << "Total accounts: " << accounts.size();
        
        double total = 0;
        for(int i=0; i<accounts.size(); i++) {
            total += accounts[i]->getBal();
        }
        
        gotoxy(30, 9);
        cout << "Total money: " << total;
    }
    
    void calcInterest() {
        for(int i=0; i<accounts.size(); i++) {
            accounts[i]->addInterest();
        }
    }
};

int main() {
    Bank b;
    b.loadAll();
    
    int choice = -1;
    system("color 0A");
    
    while(choice != 0) {
        system("cls");
        gotoxy(30, 2);
        cout << "==============================";
        gotoxy(30, 3);
        cout << "    BANK MANAGEMENT SYSTEM    ";
        gotoxy(30, 4);
        cout << "==============================";
        gotoxy(30, 5);
        cout << "1. Create Account";
        gotoxy(30, 6);
        cout << "2. Deposit";
        gotoxy(30, 7);
        cout << "3. Withdraw";
        gotoxy(30, 8);
        cout << "4. Transfer";
        gotoxy(30, 9);
        cout << "5. Show Account";
        gotoxy(30, 10);
        cout << "6. List All";
        gotoxy(30, 11);
        cout << "7. Delete Account";
        gotoxy(30, 12);
        cout << "8. Monthly Summary";
        gotoxy(30, 13);
        cout << "9. Admin Mode";
        gotoxy(30, 14);
        cout << "10. Calculate Interest";
        gotoxy(30, 15);
        cout << "11. Save";
        gotoxy(30, 16);
        cout << "12. Load";
        gotoxy(30, 17);
        cout << "0. Exit";
        gotoxy(30, 18);
        cout << "Choice: ";
        cin >> choice;
        
        switch(choice) {
            case 1: b.create(); break;
            case 2: b.doDeposit(); break;
            case 3: b.doWithdraw(); break;
            case 4: b.doTransfer(); break;
            case 5: b.showAccount(); break;
            case 6: b.listAll(); break;
            case 7: b.delAccount(); break;
            case 8: b.showMonthly(); break;
            case 9: b.admin(); break;
            case 10: b.calcInterest(); break;
            case 11: b.saveAll(); break;
            case 12: b.loadAll(); break;
            case 0: 
                gotoxy(30, 20);
                cout << "Goodbye!";
                b.saveAll();
                break;
            default:
                gotoxy(30, 20);
                cout << "Invalid!";
        }
        
        if(choice != 0) {
            gotoxy(30, 22);
            system("pause");
        }
    }
    
    return 0;
}
