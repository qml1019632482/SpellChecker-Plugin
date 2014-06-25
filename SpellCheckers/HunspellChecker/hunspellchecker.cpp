/**************************************************************************
**
** Copyright (c) 2014 Carel Combrink
**
** This file is part of the SpellChecker Plugin, a Qt Creator plugin.
**
** The SpellChecker Plugin is free software: you can redistribute it and/or 
** modify it under the terms of the GNU Lesser General Public License as 
** published by the Free Software Foundation, either version 3 of the 
** License, or (at your option) any later version.
** 
** The SpellChecker Plugin is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
** 
** You should have received a copy of the GNU Lesser General Public License
** along with the SpellChecker Plugin.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/

#include "hunspellchecker.h"
#include "HunspellConstants.h"
#include "hunspelloptionswidget.h"

#include "../../spellcheckerconstants.h"

#include <hunspell/hunspell.hxx>

#include <coreplugin/icore.h>

#include <QFile>

class SpellChecker::Checker::Hunspell::HunspellCheckerPrivate {
public:
    ::Hunspell *hunspell;
    QString dictionary;
    QString userDictionary;

    HunspellCheckerPrivate() :
        hunspell(NULL),
        dictionary(),
        userDictionary()
    {}
    ~HunspellCheckerPrivate() {}
};
//--------------------------------------------------
//--------------------------------------------------
//--------------------------------------------------

using namespace SpellChecker::Checker::Hunspell;

HunspellChecker::HunspellChecker() :
    ISpellChecker(),
    d(new HunspellCheckerPrivate())
{
    loadSettings();
    /* Get the affix dictionary path */
    QString affPath = QString(d->dictionary).replace(QRegExp(QLatin1String("\\.dic$")), QLatin1String(".aff"));
    d->hunspell = new ::Hunspell(affPath.toAscii(), d->dictionary.toAscii());
    loadUserAddedWords();
}
//--------------------------------------------------

HunspellChecker::~HunspellChecker()
{
    saveSettings();
    delete d->hunspell;
    delete d;
}
//--------------------------------------------------

QString HunspellChecker::name() const
{
    return tr("Hunspell");
}
//--------------------------------------------------

void HunspellChecker::loadSettings()
{
    QSettings* settings = Core::ICore::settings();
    settings->beginGroup(QLatin1String(Constants::CORE_SETTINGS_GROUP));
    settings->beginGroup(QLatin1String(Constants::CORE_SPELLCHECKERS_GROUP));
    settings->beginGroup(QLatin1String(SpellCheckers::HunspellChecker::Constants::SETTINGS_GROUP));
    d->dictionary = settings->value(QLatin1String(SpellCheckers::HunspellChecker::Constants::SETTING_DICTIONARY), QLatin1String("")).toString();
    d->userDictionary = settings->value(QLatin1String(SpellCheckers::HunspellChecker::Constants::SETTING_USER_DICTIONARY), QLatin1String("")).toString();
    settings->endGroup();
    settings->endGroup();
    settings->endGroup();
}
//--------------------------------------------------

void HunspellChecker::loadUserAddedWords()
{
    /* Save the word to the user dictionary */
    if(d->userDictionary.isEmpty() == true) {
        qDebug() << "loadUserAddedWords: User dictionary name empty";
        return;
    }

    QFile dictionary(d->userDictionary);
    if(dictionary.open(QIODevice::ReadOnly) == false) {
        qDebug() << "loadUserAddedWords: Could not open user dictionary file: " << d->userDictionary;
        return;
    }

    QTextStream stream(&dictionary);
    QString word;
    while(stream.atEnd() != true) {
        word = stream.readLine();
        ignoreWord(word);
    }
    dictionary.close();
}
//--------------------------------------------------

void HunspellChecker::saveSettings() const
{
    QSettings* settings = Core::ICore::settings();
    settings->beginGroup(QLatin1String(Constants::CORE_SETTINGS_GROUP));
    settings->beginGroup(QLatin1String(Constants::CORE_SPELLCHECKERS_GROUP));
    settings->beginGroup(QLatin1String(SpellCheckers::HunspellChecker::Constants::SETTINGS_GROUP));
    settings->setValue(QLatin1String(SpellCheckers::HunspellChecker::Constants::SETTING_DICTIONARY), d->dictionary);
    settings->setValue(QLatin1String(SpellCheckers::HunspellChecker::Constants::SETTING_USER_DICTIONARY), d->userDictionary);
    settings->endGroup();
    settings->endGroup();
    settings->endGroup();
    settings->sync();
}
//--------------------------------------------------

bool HunspellChecker::isSpellingMistake(const QString &word) const
{
    bool recognised = d->hunspell->spell(word.toAscii());
    return (recognised == false);
}
//--------------------------------------------------

void HunspellChecker::getSuggestionsForWord(const QString &word, QStringList &suggestionsList) const
{
    char ** suggestions;
    int numSuggestions = d->hunspell->suggest(&suggestions, word.toAscii());
    for (int i = 0; i < numSuggestions; ++i) {
        suggestionsList << QLatin1String(suggestions[i]);
    }
    d->hunspell->free_list(&suggestions, numSuggestions);
    return;
}
//--------------------------------------------------

void HunspellChecker::addWord(const QString &word)
{
    d->hunspell->add(word.toAscii());
    /* Save the word to the user dictionary */
    if(d->userDictionary.isEmpty() == true) {
        qDebug() << "User dictionary name empty";
        return;
    }

    QFile dictionary(d->userDictionary);
    if(dictionary.open(QIODevice::Append) == false) {
        qDebug() << "Could not open user dictionary file: " << d->userDictionary;
        return;
    }

    QTextStream stream(&dictionary);
    stream << word << endl;
    dictionary.close();
}
//--------------------------------------------------

void HunspellChecker::ignoreWord(const QString &word)
{
    d->hunspell->add(word.toAscii());
}
//--------------------------------------------------

QWidget *HunspellChecker::optionsWidget()
{
    HunspellOptionsWidget* widget = new HunspellOptionsWidget(d->dictionary, d->userDictionary);
    connect(this, SIGNAL(dictionaryChanged(QString)), widget, SLOT(updateDictionary(QString)));
    connect(this, SIGNAL(userDictionaryChanged(QString)), widget, SLOT(updateUserDictionary(QString)));
    connect(widget, SIGNAL(dictionaryChanged(QString)), this, SLOT(updateDictionary(QString)));
    connect(widget, SIGNAL(userDictionaryChanged(QString)), this, SLOT(updateUserDictionary(QString)));
    return widget;
}
//--------------------------------------------------

void HunspellChecker::updateDictionary(const QString &dictionary)
{
    if(d->dictionary != dictionary) {
        d->dictionary = dictionary;
        emit dictionaryChanged(d->dictionary);
    }
}
//--------------------------------------------------

void HunspellChecker::updateUserDictionary(const QString &userDictionary)
{
    if(d->userDictionary != userDictionary) {
        d->userDictionary = userDictionary;
        emit userDictionaryChanged(d->userDictionary);
    }
}
//--------------------------------------------------