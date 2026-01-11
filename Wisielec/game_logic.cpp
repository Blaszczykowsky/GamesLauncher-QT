#include "game_logic.h"
#include <QRegularExpression>
#include <QRandomGenerator>

WisielecLogic::WisielecLogic(QObject *parent)
    : QObject(parent),
    errors(0),
    maxErrors(8),
    state(GameState::WaitingForWord)
{
    dictionary << "PROGRAMOWANIE" << "KOMPUTER" << "INTERNET" << "KLAWIATURA"
               << "RZEKA" << "SAMOCHÓD" << "KSIĄŻKA" << "TELEFON"
               << "WARSZAWA" << "KRAKÓW" << "PROGRAMISTA" << "APLIKACJA"
               << "MONITOR" << "MYSZKA" << "GŁOŚNIKI" << "SŁUCHAWKI"
               << "ŻÓŁĆ" << "GĘŚ" << "CHRZĄSZCZ" << "SZCZĘŚCIE";
}

void WisielecLogic::generateRandomWord()
{
    if (dictionary.isEmpty()) return;
    int index = QRandomGenerator::global()->bounded(dictionary.size());
    setWord(dictionary[index]);
}

bool WisielecLogic::setWord(const QString &newWord)
{
    QString trimmed = newWord.trimmed().toUpper();

    if (!isValidWord(trimmed)) {
        return false;
    }

    word = trimmed;
    usedLetters.clear();
    errors = 0;

    int calculatedMax = word.length() + 2;
    if (calculatedMax < 5) calculatedMax = 5;
    if (calculatedMax > 15) calculatedMax = 15;
    maxErrors = calculatedMax;

    state = GameState::Playing;

    emit wordSet(getMaskedWord());
    emit gameStateChanged(state);
    emit errorsChanged(errors);

    return true;
}

bool WisielecLogic::guessLetter(QChar letter)
{
    if (state != GameState::Playing) {
        return false;
    }

    QChar upperLetter = letter.toUpper();

    if (!isValidLetter(upperLetter)) {
        return false;
    }

    if (usedLetters.contains(upperLetter)) {
        return false;
    }

    usedLetters.insert(upperLetter);

    bool correct = word.contains(upperLetter);

    if (!correct) {
        errors++;
        emit errorsChanged(errors);
    }

    emit letterGuessed(upperLetter, correct);
    checkWinCondition();

    return true;
}

void WisielecLogic::resetGame()
{
    word.clear();
    usedLetters.clear();
    errors = 0;
    state = GameState::WaitingForWord;

    emit gameStateChanged(state);
    emit errorsChanged(errors);
}

QString WisielecLogic::getMaskedWord() const
{
    return createMask();
}

WisielecLogic::GameState WisielecLogic::getState() const
{
    return state;
}

int WisielecLogic::getErrors() const
{
    return errors;
}

int WisielecLogic::getMaxErrors() const
{
    return maxErrors;
}

QSet<QChar> WisielecLogic::getUsedLetters() const
{
    return usedLetters;
}

QString WisielecLogic::getWord() const
{
    return word;
}

bool WisielecLogic::isValidWord(const QString &word)
{
    if (word.isEmpty()) return false;
    static QRegularExpression regex("^[A-ZĄĆĘŁŃÓŚŹŻ ]+$");
    QRegularExpressionMatch match = regex.match(word);
    return match.hasMatch();
}

bool WisielecLogic::isValidLetter(QChar letter)
{
    return letter.isLetter();
}

void WisielecLogic::checkWinCondition()
{
    if (errors >= maxErrors) {
        state = GameState::Lost;
        emit gameStateChanged(state);
        return;
    }

    for (QChar c : word) {
        if (c == ' ') continue;
        if (!usedLetters.contains(c)) {
            return;
        }
    }

    state = GameState::Won;
    emit gameStateChanged(state);
}

QString WisielecLogic::createMask() const
{
    QString mask;
    for (QChar c : word) {
        if (c == ' ') {
            mask += "  ";
        } else if (usedLetters.contains(c)) {
            mask += c;
            mask += ' ';
        } else {
            mask += "_ ";
        }
    }
    return mask.trimmed();
}
