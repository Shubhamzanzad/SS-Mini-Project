
struct Admin
{
    char userName[50];
    char password[50];
};

struct Student
{
    char userName[50];
    char password[50];
    char name[50];
    char email[50];
    char address[100];
    bool isActive;
    char gender;
    int rollNumber;
    int age;
};

struct Professor
{
    int id;
    char userName[50];
    char password[50];
    char name[50];
    char email[50];
    char address[100];
    char designation[20];
    char department[20];
};

struct Course
{
    char cname[50];
    char department[50];
    int credits;
    int seats;
    int available;
    int profId;
    int isActive;
    int id;
};

struct Enrollment
{
    int studentID;
    int courseID;
    int profID;
    int enrollmentID;
    int isEnrolled;
};

struct record
{
    int student;
    int teacher;
    int courses;
    int enrollments;
};