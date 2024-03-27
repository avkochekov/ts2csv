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

    size_t start = line.find("<source>") + 8;
    size_t end = line.find("</source>");
    string str = line.substr(start, end - start);

    translationStrings.push_back(str);
}

void processCsvFileLine(string& line, vector<string>& row, vector<vector<string>>& csvContent, string& word)
{
    row.clear();

    size_t start = -1, end = -1, lastEndIndex = 0;

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

        if (start == end) {
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

    for (string& language : languages) {
        replacementsByLanguage[language] = map<string, string>();
    }

    bool isHeader = true;

    for (vector<string>& content : csvContent) {
        if (isHeader) {
            isHeader = false;
            continue;
        }

        string sourceString = content[0];

        for (int i = 1; i < languages.size(); i++) {
            string lang = languages[i - 1];

            if (lang.empty()) {
                break;
            }

            map<string, string>* map = &replacementsByLanguage[lang];

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
    vector<string> headers = csvContent[0];
    vector<string> languages = getLanguagesFromHeaders(headers);

    map<string, map<string, string>> replacementsByLanguage = generateReplacementsByLanguageMap(csvContent, languages);

    std::ifstream t(filename);
    std::stringstream buffer;
    buffer << t.rdbuf();
    string tsFileContents = buffer.str();
    t.close();

    map<string, string> languageFiles;
    const string TRANSLATION_UNTRANSLATED_STRING = "<translation type=\"unfinished\"></translation>";
    const size_t TRANSLATION_UNTRANSLATED_STRING_LENGTH = TRANSLATION_UNTRANSLATED_STRING.length();

    for (string &language: languages) {
        map<string, string> replacements = replacementsByLanguage[language];
        string langTsFileContents = string(tsFileContents);

        for (auto it = replacements.begin(); it != replacements.end(); ++it) {
            string source = it->first;
            string translation = it->second;
            string sourceString = "<source>" +  source + "</source>";
            string translationString = "<translation>" + translation + "</translation>";
            size_t sourceStringLength = sourceString.length();
            size_t startIndex = langTsFileContents.find(sourceString) + sourceStringLength;
            size_t translationStartIndex = langTsFileContents.find(TRANSLATION_UNTRANSLATED_STRING, startIndex);
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

    string filename = string(argv[1]);
    bool isCsv = argc == 3;
    string csvFilename;

    if (argc == 3) {
        csvFilename = argv[2];
    }

    string line, word;

    string openFile = csvFilename.empty() ? filename : csvFilename;
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
