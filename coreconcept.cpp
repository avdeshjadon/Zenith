#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <set>
#include <stack>
#include <deque>
#include <queue>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <utility>

using namespace std;

// Transaction structure
struct Transaction {
    int id;
    double amount;
    string category;
    string description;
    string date;
    bool isIncome;

    Transaction(int _id, double _amount, string _category,
                string _description, string _date, bool _isIncome)
        : id(_id), amount(_amount), category(_category),
          description(_description), date(_date), isIncome(_isIncome) {}
    
    Transaction() {} // Default constructor
};

// Personal Finance Manager Class
class PersonalFinanceManager {
private:
    // DSA 1: DYNAMIC ARRAY - Store transactions
    vector<Transaction> transactions;
    
    // DSA 2: PREFIX SUM ARRAY - For O(1) balance queries
    vector<double> prefixSum;
    
    // DSA 3: HASH MAPS - For category tracking
    unordered_map<string, double> categoryExpenses;
    unordered_map<string, int> categoryCount;
    
    // DSA 4: STACK - For undo functionality
    stack<Transaction> undoStack;
    
    // DSA 5: SET (BST) - For date-based queries
    set<pair<string, int>> dateIndex;
    map<string, vector<int>> monthlyIndex;
    
    // DSA 6: DEQUE - For maintaining recent transactions
    deque<Transaction> recentTransactions;
    
    int nextId;
    double currentBalance;
    const int RECENT_LIMIT = 10;

public:
    PersonalFinanceManager() : nextId(1), currentBalance(0.0) {
        prefixSum.push_back(0.0);
    }

    // Add transaction
    string addTransaction(double amount, string category, string description,
                         string date, bool isIncome) {
        Transaction trans(nextId++, amount, category, description, date, isIncome);
        
        // DSA 1: Dynamic Array
        transactions.push_back(trans);
        
        // DSA 2: Prefix Sum - Update balance
        double balanceChange = isIncome ? amount : -amount;
        currentBalance += balanceChange;
        prefixSum.push_back(currentBalance);
        
        // DSA 3: Hash Maps - Update categories
        if (!isIncome) {
            categoryExpenses[category] += amount;
            categoryCount[category]++;
        }
        
        // DSA 4: Stack - Store for undo
        undoStack.push(trans);
        if (undoStack.size() > 5) {
            stack<Transaction> temp;
            for (int i = 0; i < 5; i++) {
                temp.push(undoStack.top());
                undoStack.pop();
            }
            while (!undoStack.empty()) undoStack.pop();
            while (!temp.empty()) {
                undoStack.push(temp.top());
                temp.pop();
            }
        }
        
        // DSA 5: Set - Date indexing
        dateIndex.insert({date, trans.id});
        string month = date.substr(0, 7);
        monthlyIndex[month].push_back(trans.id);
        
        // DSA 6: Deque - Recent transactions
        recentTransactions.push_back(trans);
        if (recentTransactions.size() > RECENT_LIMIT) {
            recentTransactions.pop_front();
        }
        
        return "SUCCESS|" + to_string(currentBalance);
    }

    // Get current balance
    string getCurrentBalance() {
        return to_string(currentBalance);
    }

    // DSA 7: HEAP - Get top expenses using priority queue
    string getTopExpenses(int k) {
        priority_queue<pair<double, string>> expenseHeap;
        for (const auto& trans : transactions) {
            if (!trans.isIncome) {
                expenseHeap.push({trans.amount, trans.description});
            }
        }
        
        string result = "";
        for (int i = 0; i < k && !expenseHeap.empty(); i++) {
            auto expense = expenseHeap.top();
            expenseHeap.pop();
            result += to_string(expense.first) + "|" + expense.second + ";";
        }
        
        return result;
    }

    // DSA 7: HEAP - Get top categories
    string getTopCategories(int k) {
        priority_queue<pair<double, string>> categoryHeap;
        for (const auto& cat : categoryExpenses) {
            categoryHeap.push({cat.second, cat.first});
        }
        
        string result = "";
        for (int i = 0; i < k && !categoryHeap.empty(); i++) {
            auto category = categoryHeap.top();
            categoryHeap.pop();
            result += category.second + "|" + to_string(category.first) + ";";
        }
        
        return result;
    }

    // DSA 8: SLIDING WINDOW - Monthly average
    string getMonthlyAverage(int months) {
    if (transactions.empty()) return "0.0";
    
    // Create a map to store monthly totals
    map<string, double> monthlyTotals;
    
    // Group expenses by month
    for (const auto& trans : transactions) {
        if (!trans.isIncome) {
            // Extract year-month from date (assuming format YYYY-MM-DD)
            string yearMonth = trans.date.substr(0, 7); // Gets "YYYY-MM"
            monthlyTotals[yearMonth] += trans.amount;
        }
    }
    
    if (monthlyTotals.empty()) return "0.0";
    
    // Calculate total expenses across all months
    double totalExpenses = 0.0;
    for (const auto& monthTotal : monthlyTotals) {
        totalExpenses += monthTotal.second;
    }
    
    // Get the number of months to consider
    int monthsToConsider = min(months, (int)monthlyTotals.size());
    
    // If we want recent months, get the last N months
    if (monthsToConsider < monthlyTotals.size()) {
        auto it = monthlyTotals.end();
        totalExpenses = 0.0;
        for (int i = 0; i < monthsToConsider; i++) {
            --it;
            totalExpenses += it->second;
        }
    }
    
    // Calculate monthly average
    double monthlyAverage = totalExpenses / monthsToConsider;
    return to_string(monthlyAverage);
}


    // Budget analysis with greedy algorithm
    string analyzeBudget(double monthlyBudget) {
        double totalSpending = 0;
        for (const auto& cat : categoryExpenses) {
            totalSpending += cat.second;
        }
        
        string result = to_string(monthlyBudget) + "|" + to_string(totalSpending) + "|";
        if (totalSpending > monthlyBudget) {
            result += "OVER|" + to_string(totalSpending - monthlyBudget) + "|";
            // DSA 9: SORTING - Greedy approach for suggestions
            vector<pair<double, string>> sortedCategories;
            for (const auto& cat : categoryExpenses) {
                sortedCategories.push_back({cat.second, cat.first});
            }
            
            sort(sortedCategories.rbegin(), sortedCategories.rend());
            for (int i = 0; i < min(3, (int)sortedCategories.size()); i++) {
                result += sortedCategories[i].second + ":" +
                         to_string(sortedCategories[i].first) + ";";
            }
        } else {
            result += "UNDER|" + to_string(monthlyBudget - totalSpending);
        }
        
        return result;
    }

    // DSA 4: Stack - Undo last transaction
    string undoLastTransaction() {
        if (undoStack.empty()) {
            return "ERROR|No transactions to undo";
        }
        
        Transaction lastTrans = undoStack.top();
        undoStack.pop();
        
        // Remove from transactions vector
        auto it = find_if(transactions.begin(), transactions.end(),
                         [&lastTrans](const Transaction& t) { return t.id == lastTrans.id; });
        if (it != transactions.end()) {
            transactions.erase(it);
        }
        
        // Update balance
        double balanceChange = lastTrans.isIncome ? -lastTrans.amount : lastTrans.amount;
        currentBalance += balanceChange;
        if (!prefixSum.empty()) {
            prefixSum.pop_back();
        }
        
        // Update categories
        if (!lastTrans.isIncome) {
            categoryExpenses[lastTrans.category] -= lastTrans.amount;
            categoryCount[lastTrans.category]--;
            if (categoryExpenses[lastTrans.category] <= 0) {
                categoryExpenses.erase(lastTrans.category);
            }
        }
        
        return "SUCCESS|" + to_string(currentBalance);
    }

    // Get all transactions
    string getAllTransactions() {
        string result = "";
        for (const auto& trans : transactions) {
            result += to_string(trans.id) + "|" +
                     to_string(trans.amount) + "|" +
                     trans.category + "|" +
                     trans.description + "|" +
                     trans.date + "|" +
                     (trans.isIncome ? "Income" : "Expense") + ";";
        }
        
        return result;
    }

    // DSA 10: PATTERN MATCHING - Fraud detection
    string detectFraud() {
        string result = "";
        
        // Check for duplicate patterns
        unordered_map<string, int> patterns;
        for (const auto& trans : transactions) {
            string pattern = to_string(trans.amount) + "|" + trans.category + "|" + trans.date;
            patterns[pattern]++;
        }
        
        bool foundIssues = false;
        for (const auto& pattern : patterns) {
            if (pattern.second > 1) {
                result += "DUPLICATE|" + pattern.first + "|" + to_string(pattern.second) + ";";
                foundIssues = true;
            }
        }
        
        // Check for large expenses
        if (!transactions.empty()) {
            vector<double> expenses;
            for (const auto& trans : transactions) {
                if (!trans.isIncome) {
                    expenses.push_back(trans.amount);
                }
            }
            
            if (!expenses.empty()) {
                sort(expenses.begin(), expenses.end());
                double median = expenses[expenses.size() / 2];
                double threshold = median * 3;
                
                for (const auto& trans : transactions) {
                    if (!trans.isIncome && trans.amount > threshold) {
                        result += "LARGE|" + to_string(trans.amount) + "|" +
                                 trans.category + "|" + trans.date + ";";
                        foundIssues = true;
                    }
                }
            }
        }
        
        if (!foundIssues) {
            result = "SAFE|No suspicious activity detected";
        }
        
        return result;
    }

    // DSA 11: SIMPLE AUTO-COMPLETE using vector search
    string getCategorySuggestions(string prefix) {
        set<string> uniqueCategories;
        for (const auto& trans : transactions) {
            if (trans.category.substr(0, prefix.length()) == prefix) {
                uniqueCategories.insert(trans.category);
            }
        }
        
        string result = "";
        for (const auto& cat : uniqueCategories) {
            result += cat + ";";
        }
        
        return result;
    }
};

// Global instance
PersonalFinanceManager pfm;

// Main function to handle commands from JavaScript
int main() {
    string command;
    while (getline(cin, command)) {
        stringstream ss(command);
        string action;
        ss >> action;
        
        if (action == "ADD") {
            double amount;
            string category, description, date, type;
            ss >> amount >> category >> description >> date >> type;
            bool isIncome = (type == "income");
            cout << pfm.addTransaction(amount, category, description, date, isIncome) << endl;
        } else if (action == "BALANCE") {
            cout << pfm.getCurrentBalance() << endl;
        } else if (action == "TOP_EXPENSES") {
            int k;
            ss >> k;
            cout << pfm.getTopExpenses(k) << endl;
        } else if (action == "TOP_CATEGORIES") {
            int k;
            ss >> k;
            cout << pfm.getTopCategories(k) << endl;
        } else if (action == "MONTHLY_AVG") {
            int months;
            ss >> months;
            cout << pfm.getMonthlyAverage(months) << endl;
        } else if (action == "BUDGET") {
            double budget;
            ss >> budget;
            cout << pfm.analyzeBudget(budget) << endl;
        } else if (action == "UNDO") {
            cout << pfm.undoLastTransaction() << endl;
        } else if (action == "TRANSACTIONS") {
            cout << pfm.getAllTransactions() << endl;
        } else if (action == "FRAUD") {
            cout << pfm.detectFraud() << endl;
        } else if (action == "SUGGEST") {
            string prefix;
            ss >> prefix;
            cout << pfm.getCategorySuggestions(prefix) << endl;
        } else if (action == "EXIT") {
            break;
        }
        
        cout.flush();
    }
    
    return 0;
}
