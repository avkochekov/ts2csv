#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <regex>


using namespace std;


void processTsFileLine(string& line, vector<string>& translationStrings)
{
    if (line.find("<source>") == string::npos) {
        return;
    }

    auto start = line.find("<source>") + 8;
    auto end = line.find("</source>");
    auto string = line.substr(start, end - start);

    translationStrings.push_back(string);
}

void processCsvFileLine(string& line, vector<string>& row, vector<vector<string>>& csvContent, string& word)
{
    row.clear();

    unsigned long long start = -1, end = -1, lastEndIndex = 0;

    while(true) {
        start = line.find('"', lastEndIndex) + 1;

        if (start == -1) {
            break;
        }

        end = line.find('"', start);
        lastEndIndex = end + 1;

        if (end == -1) {
            break;
        }

        word = line.substr(start, end - start);

        row.push_back(word);
    }

    csvContent.push_back(row);
}

void showInstructions()
{
    cout << "ts2csv project.ts"
         << "\n\nCSV to TS mode:"
         << "\nts2csv project.ts output.csv"
         << endl;
}

map<string, map<string, string>> generateReplacementsByLanguageMap(vector<vector<string>>& csvContent, vector<string>& languages)
{
    map<string, map<string, string>> replacementsByLanguage;

    for (auto& language : languages) {
        replacementsByLanguage[language] = map<string, string>();
    }

    bool isHeader = true;

    for (auto& content : csvContent) {
        if (isHeader) {
            isHeader = false;
            continue;
        }

        auto sourceString = content[0];

        for (int i = 1; i < languages.size(); i++) {
            auto lang = languages[i - 1];

            auto map = &replacementsByLanguage[lang];
            map->emplace(sourceString, content[i]);
        }
    }

    return replacementsByLanguage;
}

vector<string> getLanguagesFromHeaders(vector<string>& headers)
{
    vector<string> languages;

    for (int i = 1; i < headers.size(); i++) {
        languages.push_back(headers[i]);
    }

    return languages;
}

string do_replace( string const & in, string const & from, string const & to )
{
    return regex_replace( in, std::regex(from), to );
}

void generateTsFiles(vector<vector<string>>& csvContent, string& filename, string& csvFilename)
{
    auto headers = csvContent[0];
    auto languages = getLanguagesFromHeaders(headers);

    auto replacementsByLanguage = generateReplacementsByLanguageMap(csvContent, languages);

    std::ifstream t(filename);
    std::stringstream buffer;
    buffer << t.rdbuf();
    auto tsFileContents = buffer.str();
    t.close();

    map<string, string> languageFiles;
    const auto TRANSLATION_UNTRANSLATED_STRING = R("<translation type=\"unfinished\"></translation>");
    const auto TRANSLATION_UNTRANSLATED_STRING_LENGTH = strlen(TRANSLATION_UNTRANSLATED_STRING);

    for (auto &language: languages) {
        cout << language << endl;

        auto replacements = replacementsByLanguage[language];
        auto langTsFileContents = string(tsFileContents);

        for (auto it = replacements.begin(); it != replacements.end(); ++it) {
            auto source = it->first;
            auto translation = it->second;
            auto sourceString = "<source>" +  source + "</source>";
            auto translationString = "<translation>" + translation + "</translation>";
            auto sourceStringLength = sourceString.length();
            auto startIndex = langTsFileContents.find(sourceString) + sourceStringLength;
            auto translationStartIndex = langTsFileContents.find(TRANSLATION_UNTRANSLATED_STRING, startIndex);
            langTsFileContents = langTsFileContents.replace(translationStartIndex, TRANSLATION_UNTRANSLATED_STRING_LENGTH, translationString);
        }

        ofstream file(language + ".ts", ios::out);

        if (!file.is_open()) {
            cout << "File could not be created:  " << language << ".ts" << endl;
            exit(1);
        }

        file << langTsFileContents;
        file.close();
    }
}

void generateCsv(vector<string>& translationStrings)
{
    ofstream csvFile("output.csv", ios::out | ios::trunc);

    csvFile << "\"Source\",\"Lang1\",\"Lang2\"\n";

    if (!csvFile.is_open()) {
        cout << "Could not create file: output.csv" << endl;
        exit(1);
    }

    for (auto& translationString :translationStrings) {
        csvFile << "\"" + translationString + "\""  << R"(,"","",)" << endl;
    }

    csvFile.close();
}

int main(int argc, char *argv[])
{
    if (argc == 1) {
        showInstructions();
        return 0;
    }

    auto filename = string(argv[1]);
    bool isCsv = argc == 3;
    string csvFilename;

    if (argc == 3) {
        csvFilename = argv[2];
    }

    string line, word;

    auto openFile = csvFilename.empty() ? filename : csvFilename;
    fstream file(openFile, ios::in);

    vector<vector<string>> csvContent;
    vector<string> row;
    vector<string> translationStrings;

    if (!file.is_open()) {
        cout << "File could not be opened: " << filename << endl;
        return 1;
    }

    if(file.is_open()) {
        while(getline(file, line)) {
            if (isCsv) {
                processCsvFileLine(line, row, csvContent, word);
            } else {
                processTsFileLine(line, translationStrings);
            }
        }
    }


    if (isCsv) {
        if (csvContent.empty()) {
            cout << "No content found!" << endl;
            return 0;
        }

        generateTsFiles(csvContent, filename, csvFilename);

    } else {
        if (translationStrings.empty()) {
            cout << "No content found!" << endl;
            return 0;
        }

        generateCsv(translationStrings);
    }

    return 0;
}
