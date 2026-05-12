#ifndef LANGUAGE_DETECTOR_H
#define LANGUAGE_DETECTOR_H

#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <set>
#include <memory>
#include <cmath>
#include <fstream>

enum class Language
{
    ROMANIAN = 0,
    ENGLISH = 1,
    GERMAN = 2,
    HUNGARIAN = 3,
    TURKISH = 4,
    DUTCH = 5,
    UNKNOWN = 6
};

enum class DetectionMethod
{
    CHARACTER_DIACRITICS_ONLY,
    ALL_METHODS
};

struct LanguageData
{
    std::map<wchar_t, int> charFreq;
    std::map<std::wstring, int> bigramFreq;
    std::set<wchar_t> diacritics;
    std::wstring name;

    // Precomputed CLR vectors for fast distance calculation
    std::vector<double> clrCharFreq;
    std::vector<double> clrBigramFreq;
};

struct DatabaseRecord
{
    std::string code;
    std::string expected;
    std::string detected;
    bool isCorrect;
};

class LanguageDetector
{
private:
    static const int POSITION_ARRAY_SIZE = 30;
    std::vector<LanguageData> languages;

    // Global sets to ensure vectors are identically sized for Euclidean distance
    std::set<wchar_t> globalAlphabet;
    std::set<std::wstring> globalBigrams;

    std::unordered_map<wchar_t, std::vector<int>> freqMap;
    std::map<std::wstring, int> textBigrams;
    double totalChars = 0;

    // Database
    std::map<std::string, DatabaseRecord> database;
    const std::string DB_FILENAME = "language_database.csv";

    void loadLanguageDataFromCSV(const std::string &filename);
    void buildGlobalSets();
    void precomputeLanguageCLRs();

    void analyzeText(const std::wstring &text);
    std::map<std::wstring, int> extractBigrams(const std::wstring &line);
    
    // Math & CoDA Functions handling Zero Imputation
    std::vector<double> calculateCLR(const std::map<wchar_t, int> &frequencies, const std::set<wchar_t> &globalSet);
    std::vector<double> calculateCLR(const std::map<std::wstring, int> &frequencies, const std::set<std::wstring> &globalSet);

    double calculateEuclideanDistance(const std::vector<double> &v1, const std::vector<double> &v2);
    double calculateDiacriticScore(const std::set<wchar_t> &diacritics);

public:
    LanguageDetector();
    Language detectLanguage(const std::wstring &text, DetectionMethod method = DetectionMethod::ALL_METHODS);
    Language detectLanguageFromFile(const std::string &filename, DetectionMethod method = DetectionMethod::ALL_METHODS, bool isTestMode = false);

    // Database Management
    void loadDatabase();
    void saveToDatabase(const std::string &title, const std::string &author, Language expected, Language detected);
    bool doesEntryExist(const std::string &code);
    void displayDatabaseAccuracy();

    // Test mode functionality
    void runTestMode();
    void processTestFile(const std::string &filename);
    Language parseExpectedLanguage(const std::string &langStr);
    std::wstring languageToString(Language lang);

    // Folder test mode functionality
    void runFolderTestMode();
    void processTestFolder(const std::string &folderPath);
    std::vector<std::string> getFilesInFolder(const std::string &folderPath);

    // Utility functions
    void reset();
    std::string readFile(const std::string &filename);
    Language detectLanguageAllMethods(const std::string &text);
    Language detectLanguageCharDiacriticsOnly(const std::string &text);
    std::string getLanguageName(Language lang);
};

#endif // LANGUAGE_DETECTOR_H