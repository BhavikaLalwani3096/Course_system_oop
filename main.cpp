#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <stdexcept>
#include <algorithm>

using namespace std;

// ========================= DATE STRUCT =========================
struct Date {
    int year, month, day;
    Date() : year(0), month(0), day(0) {}
    Date(int y, int m, int d) : year(y), month(m), day(d) {}
    string toString() const {
        ostringstream ss;
        ss << year << '-' << setw(2) << setfill('0') << month << '-' << setw(2) << setfill('0') << day;
        return ss.str();
    }
};

// Simple hash for password
string _hash_password(const string& password) {
    hash<string> hasher;
    size_t h = hasher(password);
    ostringstream ss;
    ss << hex << h;
    return ss.str();
}

// Forward declarations
class Course;
class Enrollment;
class Student;
class Instructor;

// ========================= USER BASE CLASS =========================
class User {
protected:
    static int _user_counter;
    static map<int, User*> _all_users;

    int userId;
    string username;
    string email;
    string password;
    string createdAt;
    bool _is_logged_in;

public:
    User(const string& uname, const string& mail, const string& pass) {
        userId = _user_counter++;
        username = uname;
        email = mail;
        password = _hash_password(pass);
        time_t now = time(nullptr);
        tm* ltm = localtime(&now);
        ostringstream ss;
        ss << (ltm->tm_year + 1900) << "-" << (ltm->tm_mon + 1) << "-" << ltm->tm_mday;
        createdAt = ss.str();
        _is_logged_in = false;
        _all_users[userId] = this;
    }

    virtual ~User() = default;

    string getHashedPassword() const { return password; }

    bool login(const string& pass) {
        if (_hash_password(pass) == password) {
            _is_logged_in = true;
            cout << "✓ Welcome back, " << username << "!" << endl;
            return true;
        }
        cout << "✗ Invalid password!" << endl;
        return false;
    }

    void logout() {
        _is_logged_in = false;
        cout << "✓ " << username << " logged out successfully!" << endl;
    }

    bool validateCredentials(const string& user_or_email, const string& pass) {
        return (username == user_or_email || email == user_or_email) &&
               _hash_password(pass) == password;
    }

    int getUserId() const { return userId; }
    string getUsername() const { return username; }
    string getEmail() const { return email; }
};

int User::_user_counter = 1;
map<int, User*> User::_all_users;

// ========================= STUDENT CLASS =========================
class Student : public User {
public:
    static int _student_counter;
    static map<int, Student*> _all_students;

    int studentId;
    string firstName;
    string lastName;
    Date dateOfBirth;
    string phoneNumber;
    vector<Enrollment*> enrollments;

    Student(int userId, const string& fName, const string& lName,
            const Date& dob, const string& phone);

    string getFullName() const { return firstName + " " + lastName; }
    void viewAvailableCourses();
    void enrollInCourse();                    // NEW: shows list first, then lets user pick ID
    void viewMyCourses();                     // NEW: shows only courses student is enrolled in
    bool enrollInCourse(int courseId);        // internal helper
};

int Student::_student_counter = 1;
map<int, Student*> Student::_all_students;

// ========================= INSTRUCTOR CLASS =========================
class Instructor : public User {
public:
    static int _instructor_counter;
    static map<int, Instructor*> _all_instructors;

    int instructorId;
    string firstName;
    string lastName;
    string expertise;
    float rating;
    string qualification;

    Instructor(int userId, const string& fName, const string& lName,
               const string& exp, const string& qual);

    void createCourse();
    void viewCourses();
};

int Instructor::_instructor_counter = 1;
map<int, Instructor*> Instructor::_all_instructors;

// ========================= COURSE =========================
class Course {
public:
    static map<int, Course*> _all_courses;

    int courseId;
    string courseTitle;
    string description;
    string difficulty;
    int durationWeeks;
    string language;
    float price;
    int instructorId;
    string instructorName;

    Course(const string& title, const string& desc, const string& diff,
           int duration, const string& lang, float price, int instrId, const string& instrName);

    void display() const;
};

map<int, Course*> Course::_all_courses;

Course::Course(const string& title, const string& desc, const string& diff,
               int duration, const string& lang, float price, int instrId, const string& instrName) {
    courseId = static_cast<int>(_all_courses.size()) + 1;
    courseTitle = title;
    description = desc;
    difficulty = diff;
    durationWeeks = duration;
    language = lang;
    this->price = price;
    instructorId = instrId;
    instructorName = instrName;

    _all_courses[courseId] = this;
}

void Course::display() const {
    cout << courseId << ". " << courseTitle << endl;
    cout << "   Instructor : " << instructorName << endl;
    cout << "   Description: " << description << endl;
    cout << "   Difficulty : " << difficulty << endl;
    cout << "   Duration   : " << durationWeeks << " weeks" << endl;
    cout << "   Language   : " << language << endl;
    cout << "   Price      : $" << fixed << setprecision(2) << price << endl;
    cout << string(50, '-') << endl;
}

// ========================= ENROLLMENT =========================
class Enrollment {
public:
    static map<int, Enrollment*> _all_enrollments;
    int enrollmentId;
    int studentId;
    int courseId;
    float progressPercentage = 0.0f;
    string completionStatus = "Not Started";

    Enrollment(int sId, int cId);
};

map<int, Enrollment*> Enrollment::_all_enrollments;

Enrollment::Enrollment(int sId, int cId) {
    enrollmentId = static_cast<int>(_all_enrollments.size()) + 1;
    studentId = sId;
    courseId = cId;
    _all_enrollments[enrollmentId] = this;
}

// ========================= STUDENT IMPLEMENTATION =========================
Student::Student(int userId, const string& fName, const string& lName,
                 const Date& dob, const string& phone) : User("", "", "") {
    auto it = User::_all_users.find(userId);
    if (it == User::_all_users.end()) 
        throw runtime_error("User ID not found");

    User* u = it->second;

    this->username = u->getUsername();
    this->email    = u->getEmail();
    this->password = u->getHashedPassword();
    this->userId   = userId;

    studentId = _student_counter++;
    firstName = fName;
    lastName = lName;
    dateOfBirth = dob;
    phoneNumber = phone;

    _all_students[studentId] = this;
}

void Student::viewAvailableCourses() {
    if (Course::_all_courses.empty()) {
        cout << "No courses available at the moment.\n";
        return;
    }
    cout << "\n=== Available Courses ===\n";
    for (auto& p : Course::_all_courses) {
        p.second->display();
    }
}

void Student::enrollInCourse() {
    viewAvailableCourses();                     // ← First show all courses with IDs
    if (Course::_all_courses.empty()) return;

    cout << "\nEnter Course ID to enroll: ";
    int cid;
    cin >> cid;
    cin.ignore(); // clear newline

    enrollInCourse(cid);                        // internal call
}

bool Student::enrollInCourse(int courseId) {
    // Check if course exists
    if (Course::_all_courses.find(courseId) == Course::_all_courses.end()) {
        cout << "✗ Course ID " << courseId << " not found!\n";
        return false;
    }

    // Check if already enrolled
    for (auto* e : enrollments) {
        if (e->courseId == courseId) {
            cout << "✗ You are already enrolled in this course!\n";
            return false;
        }
    }

    // Create and store enrollment
    Enrollment* newEnroll = new Enrollment(studentId, courseId);
    enrollments.push_back(newEnroll);

    string title = Course::_all_courses[courseId]->courseTitle;
    cout << "✓ Successfully enrolled in: " << title << " (ID: " << courseId << ")\n";
    return true;
}

void Student::viewMyCourses() {
    if (enrollments.empty()) {
        cout << "You are not enrolled in any courses yet.\n";
        return;
    }

    cout << "\n=== My Enrolled Courses ===\n";
    for (auto* enroll : enrollments) {
        auto it = Course::_all_courses.find(enroll->courseId);
        if (it != Course::_all_courses.end()) {
            it->second->display();
            cout << "   Progress   : " << enroll->progressPercentage << "%" << endl;
            cout << "   Status     : " << enroll->completionStatus << endl;
            cout << string(50, '=') << endl;
        }
    }
}

// ========================= INSTRUCTOR IMPLEMENTATION =========================
Instructor::Instructor(int userId, const string& fName, const string& lName,
                       const string& exp, const string& qual) : User("", "", "") {
    auto it = User::_all_users.find(userId);
    if (it == User::_all_users.end()) 
        throw runtime_error("User ID not found");

    User* u = it->second;

    this->username = u->getUsername();
    this->email    = u->getEmail();
    this->password = u->getHashedPassword();
    this->userId   = userId;

    instructorId = _instructor_counter++;
    firstName = fName;
    lastName = lName;
    expertise = exp;
    qualification = qual;
    rating = 0.0f;

    _all_instructors[instructorId] = this;
}

void Instructor::createCourse() {
    string title, desc, diff, lang;
    int duration;
    float price;

    cout << "\n=== Create New Course ===\n";
    cout << "Enter Course Title: "; getline(cin, title);
    if (title.empty()) { cout << "Course title cannot be empty!\n"; return; }

    cout << "Enter Description: "; getline(cin, desc);
    cout << "Enter Difficulty (Beginner/Intermediate/Advanced): "; getline(cin, diff);
    cout << "Enter Duration (in weeks): "; cin >> duration; cin.ignore();
    cout << "Enter Language: "; getline(cin, lang);
    cout << "Enter Price ($): "; cin >> price; cin.ignore();

    string instrFullName = firstName + " " + lastName;
    Course* newCourse = new Course(title, desc, diff, duration, lang, price, instructorId, instrFullName);

    cout << "✓ Course '" << title << "' created successfully!\n";
}

void Instructor::viewCourses() {
    if (Course::_all_courses.empty()) {
        cout << "No courses available.\n";
        return;
    }
    cout << "\n=== All Courses ===\n";
    for (auto& p : Course::_all_courses) {
        p.second->display();
    }
}

// ========================= SAMPLE DATA & MENUS =========================
void initialize_sample_data() {
    User* u1 = new User("john_student", "john@email.com", "password123");
    User* u2 = new User("jane_instructor", "jane@email.com", "password123");
    User* u3 = new User("bob_instructor", "bob@email.com", "password123");

    Student* s1 = new Student(u1->getUserId(), "John", "Doe", Date(2000,5,15), "+1234567890");
    Instructor* i1 = new Instructor(u2->getUserId(), "Jane", "Smith", "Machine Learning", "PhD");
    Instructor* i2 = new Instructor(u3->getUserId(), "Bob", "Wilson", "Web Development", "M.Tech");

    cout << "\n✓ Sample data initialized successfully!" << endl;
    cout << "  Student     : john_student / password123" << endl;
    cout << "  Instructor 1: jane_instructor / password123" << endl;
    cout << "  Instructor 2: bob_instructor / password123" << endl;
}

void print_header(const string& title) {
    cout << "\n" << string(60, '=') << endl;
    cout << "  " << title << endl;
    cout << string(60, '=') << endl;
}

void student_menu(Student* student) {
    while (true) {
        print_header("STUDENT MENU - Welcome " + student->getFullName());
        cout << "1. View Available Courses\n";
        cout << "2. Enroll in a Course\n";
        cout << "3. View My Courses\n";
        cout << "0. Logout\n";
        cout << "Enter choice: ";

        string ch;
        getline(cin, ch);

        if (ch == "0") {
            student->logout();
            break;
        }
        else if (ch == "1") {
            student->viewAvailableCourses();
        }
        else if (ch == "2") {
            student->enrollInCourse();           // shows list first, then asks for ID
        }
        else if (ch == "3") {
            student->viewMyCourses();
        }
        else {
            cout << "Invalid option!\n";
        }
    }
}

void instructor_menu(Instructor* instructor) {
    while (true) {
        print_header("INSTRUCTOR MENU - Welcome " + instructor->firstName + " " + instructor->lastName);
        cout << "1. Create New Course\n2. View All Courses\n0. Logout\nEnter choice: ";
        string ch;
        getline(cin, ch);

        if (ch == "0") {
            instructor->logout();
            break;
        }
        else if (ch == "1") {
            instructor->createCourse();
        }
        else if (ch == "2") {
            instructor->viewCourses();
        }
        else {
            cout << "Invalid option!\n";
        }
    }
}

void main_menu() {
    initialize_sample_data();

    while (true) {
        print_header("ONLINE LEARNING MANAGEMENT SYSTEM");
        cout << "\n1. Student Login\n2. Instructor Login\n0. Exit\n";
        string choice;
        getline(cin, choice);

        if (choice == "1") {
            string user, pass;
            cout << "Username/Email: "; getline(cin, user);
            cout << "Password: "; getline(cin, pass);

            bool loggedIn = false;
            for (auto& p : Student::_all_students) {
                if (p.second->validateCredentials(user, pass)) {
                    p.second->login(pass);
                    student_menu(p.second);
                    loggedIn = true;
                    break;
                }
            }
            if (!loggedIn) cout << "✗ Invalid username/email or password for Student!\n";
        }
        else if (choice == "2") {
            string user, pass;
            cout << "Username/Email: "; getline(cin, user);
            cout << "Password: "; getline(cin, pass);

            bool loggedIn = false;
            for (auto& p : Instructor::_all_instructors) {
                if (p.second->validateCredentials(user, pass)) {
                    p.second->login(pass);
                    instructor_menu(p.second);
                    loggedIn = true;
                    break;
                }
            }
            if (!loggedIn) cout << "✗ Invalid username/email or password for Instructor!\n";
        }
        else if (choice == "0") {
            cout << "\nThank you for using the Learning Management System!\n";
            break;
        }
        else {
            cout << "Invalid choice! Please try again.\n";
        }
    }
}

int main() {
    try {
        main_menu();
    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
    }
    return 0;
}