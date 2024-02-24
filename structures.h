
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

struct Professor {
    int id;
    char userName[50];
    char password[50];
    char name[50];
    char email[50];
    char address[100];
    char designation[20];
    char department[20];
};