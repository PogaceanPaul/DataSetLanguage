#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QString>
#include <fstream>
#include <string>
#include "language_detector.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    LanguageDetector detector;

    // Fereastra principală
    QWidget window;
    window.setWindowTitle("Language Detector (CoDA) - Interfață Grafică");
    window.resize(750, 550);

    QVBoxLayout *mainLayout = new QVBoxLayout(&window);

    // Rândul 1: Titlu și Autor
    QHBoxLayout *metaLayout = new QHBoxLayout();
    QLineEdit *titleEdit = new QLineEdit();
    titleEdit->setPlaceholderText("Ex: Luceafarul (Doar pt. Salvare)");
    QLineEdit *authorEdit = new QLineEdit();
    authorEdit->setPlaceholderText("Ex: Mihai_Eminescu (Doar pt. Salvare)");

    metaLayout->addWidget(new QLabel("Titlu:"));
    metaLayout->addWidget(titleEdit);
    metaLayout->addWidget(new QLabel("Autor:"));
    metaLayout->addWidget(authorEdit);
    mainLayout->addLayout(metaLayout);

    // Rândul 2: Limba Așteptată și Butonul de Statistici
    QHBoxLayout *langLayout = new QHBoxLayout();
    QComboBox *langCombo = new QComboBox();
    langCombo->addItem("Romanian");  // Index 0
    langCombo->addItem("English");   // Index 1
    langCombo->addItem("German");    // Index 2
    langCombo->addItem("Hungarian"); // Index 3
    langCombo->addItem("Turkish");   // Index 4
    langCombo->addItem("Dutch");     // Index 5

    QPushButton *accuracyBtn = new QPushButton("📊 Vezi Acuratețea Bazei de Date");
    accuracyBtn->setStyleSheet("background-color: #3498db; color: white; font-weight: bold; border-radius: 5px; padding: 5px 15px;");

    langLayout->addWidget(new QLabel("Limba Așteptată:"));
    langLayout->addWidget(langCombo);
    langLayout->addStretch();
    langLayout->addWidget(accuracyBtn);
    mainLayout->addLayout(langLayout);

    // Rândul 3: Zona de Paste pentru Text
    mainLayout->addWidget(new QLabel("Introduceți textul aici:"));
    QTextEdit *textEdit = new QTextEdit();
    mainLayout->addWidget(textEdit);

    // Rândul 4: Butoanele de Acțiune (Testare vs Salvare)
    QHBoxLayout *buttonsLayout = new QHBoxLayout();

    QPushButton *quickTestBtn = new QPushButton("🔍 Testare Rapidă (Fără Salvare)");
    quickTestBtn->setMinimumHeight(40);
    quickTestBtn->setStyleSheet("font-weight: bold;");

    QPushButton *analyzeAndSaveBtn = new QPushButton("💾 Analizează și Salvează în DB");
    analyzeAndSaveBtn->setMinimumHeight(40);
    analyzeAndSaveBtn->setStyleSheet("font-weight: bold;");

    buttonsLayout->addWidget(quickTestBtn);
    buttonsLayout->addWidget(analyzeAndSaveBtn);
    mainLayout->addLayout(buttonsLayout);

    // Rândul 5: Rezultatul
    QLabel *resultLabel = new QLabel("Rezultat: Așteptare text...");
    resultLabel->setStyleSheet("font-weight: bold; color: #2c3e50; font-size: 15px; margin-top: 10px;");
    mainLayout->addWidget(resultLabel);

    // --- LOGICA PENTRU TEST RAPID ---
    QObject::connect(quickTestBtn, &QPushButton::clicked, [&]()
                     {
        QString text = textEdit->toPlainText();
        if (text.isEmpty()) {
            QMessageBox::warning(&window, "Eroare", "Te rog să introduci un text pentru a fi testat!");
            return;
        }

        std::string stdText = text.toStdString();
        Language detectedLang = detector.detectLanguageAllMethods(stdText);
        
        QString detName = QString::fromStdString(detector.getLanguageName(detectedLang));
        resultLabel->setStyleSheet("font-weight: bold; color: #d35400; font-size: 15px; margin-top: 10px;");
        resultLabel->setText("🔍 Test Rapid: Limba detectată este " + detName + " (Nu a fost salvat)"); });

    // --- LOGICA PENTRU SALVARE ---
    QObject::connect(analyzeAndSaveBtn, &QPushButton::clicked, [&]()
                     {
        QString title = titleEdit->text();
        QString author = authorEdit->text();
        QString text = textEdit->toPlainText();
        int expectedIdx = langCombo->currentIndex();

        if (title.isEmpty() || author.isEmpty() || text.isEmpty()) {
            QMessageBox::warning(&window, "Eroare", "Titlul, Autorul și Textul sunt obligatorii pentru salvare!");
            return;
        }

        Language expectedLang = static_cast<Language>(expectedIdx);
        std::string stdText = text.toStdString();
        
        Language detectedLang = detector.detectLanguageAllMethods(stdText);
        detector.saveToDatabase(title.toStdString(), author.toStdString(), expectedLang, detectedLang);
        
        QString detName = QString::fromStdString(detector.getLanguageName(detectedLang));
        resultLabel->setStyleSheet("font-weight: bold; color: #27ae60; font-size: 15px; margin-top: 10px;");
        resultLabel->setText("✅ Salvat cu succes! Limba detectată: " + detName); });

    // --- LOGICA PENTRU ACURATEȚE (CITIRE DIN CSV) ---
    QObject::connect(accuracyBtn, &QPushButton::clicked, [&]()
                     {
        std::ifstream file("language_database.csv");
        if (!file.is_open()) {
            QMessageBox::information(&window, "Bază de date", "Baza de date este goală sau nu a fost creată încă.");
            return;
        }

        int total = 0;
        int corecte = 0;
        std::string line;

        while (std::getline(file, line)) {
            if (line.empty()) continue;
            total++;
            // Verificăm ultimul caracter de pe rând (0 sau 1)
            if (line.back() == '1') {
                corecte++;
            }
        }

        if (total == 0) {
            QMessageBox::information(&window, "Bază de date", "Baza de date nu conține nicio înregistrare validă.");
        } else {
            double procent = (static_cast<double>(corecte) / total) * 100.0;
            QString mesaj = QString("=== STATISTICI CODA ===\n\n"
                                    "Total texte analizate: %1\n"
                                    "Predicții corecte: %2\n\n"
                                    "Acuratețe globală: %3%")
                                    .arg(total).arg(corecte).arg(QString::number(procent, 'f', 2));
            
            QMessageBox::information(&window, "Acuratețe Sistem", mesaj);
        } });

    window.show();
    return app.exec();
}