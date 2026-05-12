#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#include "language_detector.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <iomanip>
#include <cwctype>
#include <codecvt>
#include <locale>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

LanguageDetector::LanguageDetector()
{
    loadLanguageDataFromCSV("frecvente_limbi.csv");
    buildGlobalSets();
    precomputeLanguageCLRs();
    loadDatabase();
}

void LanguageDetector::loadLanguageDataFromCSV(const std::string &filename)
{
    languages.clear();
    languages.resize(6);

    languages[0].name = L"Romanian";
    languages[1].name = L"English";
    languages[2].name = L"German";
    languages[3].name = L"Hungarian";
    languages[4].name = L"Turkish";
    languages[5].name = L"Dutch";

    auto getLangIndex = [](const std::wstring &name) -> int
    {
        if (name == L"Romanian")
            return 0;
        if (name == L"English")
            return 1;
        if (name == L"German")
            return 2;
        if (name == L"Hungarian")
            return 3;
        if (name == L"Turkish")
            return 4;
        if (name == L"Dutch")
            return 5;
        return -1;
    };

    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "EROARE CRITICĂ: Nu s-a putut deschide " << filename << std::endl;
        std::cerr << "Asigura-te ca fisierul frecvente_limbi.csv se afla langa executabil!" << std::endl;
        return;
    }

    std::string line;
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

    while (std::getline(file, line))
    {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        if (line.empty())
            continue;

        std::wstring wline = converter.from_bytes(line);
        std::wstringstream wss(wline);

        std::wstring langName, type, key, valueStr;

        std::getline(wss, langName, L',');
        std::getline(wss, type, L',');
        std::getline(wss, key, L',');
        std::getline(wss, valueStr, L',');

        int idx = getLangIndex(langName);
        if (idx == -1)
            continue;

        int value = (!valueStr.empty()) ? std::stoi(valueStr) : 0;

        if (type == L"C")
        {
            languages[idx].charFreq[key[0]] = value;
        }
        else if (type == L"B")
        {
            languages[idx].bigramFreq[key] = value;
        }
        else if (type == L"D")
        {
            languages[idx].diacritics.insert(key[0]);
        }
    }
}

void LanguageDetector::buildGlobalSets()
{
    for (const auto &lang : languages)
    {
        for (const auto &pair : lang.charFreq)
            globalAlphabet.insert(pair.first);
        for (const auto &pair : lang.bigramFreq)
            globalBigrams.insert(pair.first);
    }
}

void LanguageDetector::precomputeLanguageCLRs()
{
    for (auto &lang : languages)
    {
        lang.clrCharFreq = calculateCLR(lang.charFreq, globalAlphabet);
        lang.clrBigramFreq = calculateCLR(lang.bigramFreq, globalBigrams);
    }
}

// Varianta corectă CoDA pentru Caractere (cu Netezire Laplace)
std::vector<double> LanguageDetector::calculateCLR(const std::map<wchar_t, int> &frequencies, const std::set<wchar_t> &globalSet)
{
    std::vector<double> clr;
    double delta = 0.5; // Laplace pseudo-count pentru a evita log(0)
    double logSum = 0.0;

    // 1. Calculăm logaritmul mediei geometrice a TUTUROR elementelor (incluzând zero-urile netezite)
    for (const auto &item : globalSet)
    {
        auto it = frequencies.find(item);
        double val = (it != frequencies.end()) ? static_cast<double>(it->second) : 0.0;
        val += delta; // Netezirea Laplace
        logSum += std::log(val);
    }

    double gm_log = logSum / globalSet.size();

    // 2. Aplicăm transformarea Centered Log-Ratio
    for (const auto &item : globalSet)
    {
        auto it = frequencies.find(item);
        double val = (it != frequencies.end()) ? static_cast<double>(it->second) : 0.0;
        val += delta;
        clr.push_back(std::log(val) - gm_log);
    }

    return clr;
}

// Varianta corectă CoDA pentru Bigrame (cu Netezire Laplace)
std::vector<double> LanguageDetector::calculateCLR(const std::map<std::wstring, int> &frequencies, const std::set<std::wstring> &globalSet)
{
    std::vector<double> clr;
    double delta = 0.5; // Laplace pseudo-count
    double logSum = 0.0;

    for (const auto &item : globalSet)
    {
        auto it = frequencies.find(item);
        double val = (it != frequencies.end()) ? static_cast<double>(it->second) : 0.0;
        val += delta;
        logSum += std::log(val);
    }

    double gm_log = logSum / globalSet.size();

    for (const auto &item : globalSet)
    {
        auto it = frequencies.find(item);
        double val = (it != frequencies.end()) ? static_cast<double>(it->second) : 0.0;
        val += delta;
        clr.push_back(std::log(val) - gm_log);
    }

    return clr;
}

double LanguageDetector::calculateEuclideanDistance(const std::vector<double> &v1, const std::vector<double> &v2)
{
    // Calculăm Distanța Aitchison (Norma Euclidiană pe vectori CLR)
    double sumSq = 0.0;
    for (size_t i = 0; i < v1.size(); ++i)
    {
        double diff = v1[i] - v2[i];
        sumSq += diff * diff;
    }
    return std::sqrt(sumSq);
}

void LanguageDetector::reset()
{
    freqMap.clear();
    textBigrams.clear();
    totalChars = 0;
}

void LanguageDetector::analyzeText(const std::wstring &text)
{
    reset();

    std::wstring lowerText = text;
    for (wchar_t &c : lowerText)
    {
        c = std::towlower(c);
    }

    std::wstring cleanText = L"";

    // 1. Curățăm textul și numărăm literele
    for (size_t i = 0; i < lowerText.size(); ++i)
    {
        wchar_t ch = lowerText[i];
        if (iswspace(ch) || iswpunct(ch))
        {
            cleanText += L" "; // Păstrăm un spațiu pentru a delimita cuvintele
            continue;
        }

        totalChars++;
        freqMap[ch].push_back(1);
        cleanText += ch;
    }

    // 2. Extragem bigramele DOAR în interiorul cuvintelor (ignorăm spațiile)
    std::wstringstream wss(cleanText);
    std::wstring word;
    while (wss >> word)
    {
        if (word.size() < 2)
            continue;
        for (size_t i = 0; i < word.size() - 1; ++i)
        {
            textBigrams[word.substr(i, 2)]++;
        }
    }
}

double LanguageDetector::calculateDiacriticScore(const std::set<wchar_t> &diacritics)
{
    if (diacritics.empty() || totalChars == 0)
        return 0.0;
    int matches = 0;
    for (wchar_t d : diacritics)
    {
        if (freqMap.find(d) != freqMap.end())
            matches += freqMap[d].size();
    }
    return static_cast<double>(matches);
}

Language LanguageDetector::detectLanguage(const std::wstring &text, DetectionMethod method)
{
    analyzeText(text);
    if (totalChars == 0)
        return Language::UNKNOWN;

    std::map<wchar_t, int> flatFreqs;
    for (const auto &entry : freqMap)
        flatFreqs[entry.first] = entry.second.size();

    std::vector<double> textCharCLR = calculateCLR(flatFreqs, globalAlphabet);
    std::vector<double> textBigramCLR = calculateCLR(textBigrams, globalBigrams);

    std::vector<double> distances(languages.size(), 0.0);

    for (size_t i = 0; i < languages.size(); ++i)
    {
        double dist = calculateEuclideanDistance(textCharCLR, languages[i].clrCharFreq);

        if (method == DetectionMethod::ALL_METHODS)
        {
            dist += calculateEuclideanDistance(textBigramCLR, languages[i].clrBigramFreq);
        }

        double diacriticMatches = calculateDiacriticScore(languages[i].diacritics);
        double decayFactor = 1.0 / (1.0 + (0.01 * totalChars));
        dist -= (diacriticMatches * decayFactor * 5.0);

        distances[i] = dist;
    }

    auto minIt = std::min_element(distances.begin(), distances.end());
    return static_cast<Language>(std::distance(distances.begin(), minIt));
}

void LanguageDetector::loadDatabase()
{
    std::ifstream file(DB_FILENAME);
    if (!file.is_open())
        return;

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream ss(line);
        std::string code, expected, detected, correctStr;

        if (std::getline(ss, code, ',') && std::getline(ss, expected, ',') &&
            std::getline(ss, detected, ',') && std::getline(ss, correctStr))
        {
            bool isCorrect = (correctStr == "1");
            database[code] = {code, expected, detected, isCorrect};
        }
    }
}

void LanguageDetector::saveToDatabase(const std::string &title, const std::string &author, Language expected, Language detected)
{
    std::string finalCode = title + "_" + author;
    std::transform(finalCode.begin(), finalCode.end(), finalCode.begin(), ::tolower);

    if (doesEntryExist(finalCode))
        return;

    bool isCorrect = (expected == detected);
    std::ofstream file(DB_FILENAME, std::ios::app);
    if (file.is_open())
    {
        file << finalCode << "," << getLanguageName(expected) << ","
             << getLanguageName(detected) << "," << (isCorrect ? "1" : "0") << "\n";
        database[finalCode] = {finalCode, getLanguageName(expected), getLanguageName(detected), isCorrect};
    }
}

bool LanguageDetector::doesEntryExist(const std::string &code)
{
    return database.find(code) != database.end();
}

void LanguageDetector::displayDatabaseAccuracy()
{
    // Implementation not strictly needed for GUI since we display it directly there
}

std::wstring LanguageDetector::languageToString(Language lang)
{
    if (lang >= Language::ROMANIAN && lang <= Language::DUTCH)
        return languages[static_cast<int>(lang)].name;
    return L"Unknown";
}

std::string LanguageDetector::getLanguageName(Language lang)
{
    std::wstring wname = languageToString(lang);
    return std::string(wname.begin(), wname.end());
}

Language LanguageDetector::detectLanguageAllMethods(const std::string &text)
{
    std::wstring wtext;
    try
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        wtext = converter.from_bytes(text);
    }
    catch (...)
    {
        wtext.assign(text.begin(), text.end());
    }
    return detectLanguage(wtext, DetectionMethod::ALL_METHODS);
}

Language LanguageDetector::detectLanguageCharDiacriticsOnly(const std::string &text)
{
    std::wstring wtext;
    try
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        wtext = converter.from_bytes(text);
    }
    catch (...)
    {
        wtext.assign(text.begin(), text.end());
    }
    return detectLanguage(wtext, DetectionMethod::CHARACTER_DIACRITICS_ONLY);
}

std::string LanguageDetector::readFile(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
        return "";
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

Language LanguageDetector::parseExpectedLanguage(const std::string &str)
{
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower == "romanian" || lower == "0")
        return Language::ROMANIAN;
    if (lower == "english" || lower == "1")
        return Language::ENGLISH;
    if (lower == "german" || lower == "2")
        return Language::GERMAN;
    if (lower == "hungarian" || lower == "3")
        return Language::HUNGARIAN;
    if (lower == "turkish" || lower == "4")
        return Language::TURKISH;
    if (lower == "dutch" || lower == "5")
        return Language::DUTCH;
    return Language::UNKNOWN;
}

void LanguageDetector::runTestMode() {}
void LanguageDetector::runFolderTestMode() {}