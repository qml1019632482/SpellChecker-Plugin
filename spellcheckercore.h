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

#ifndef SPELLCHECKERCORE_H
#define SPELLCHECKERCORE_H

#include "Word.h"

#include <projectexplorer/project.h>
#include <coreplugin/editormanager/editormanager.h>

#include <QObject>
#include <QSettings>

namespace Core { class IOptionsPage; }

namespace SpellChecker {
namespace Internal {
class SpellCheckerCorePrivate;
class OutputPane;
}
class IDocumentParser;
class ISpellChecker;

/*!
 * \brief The SpellCheckerCore class
 *
 * This is the core class of the spell checker. It handles document parser, spell
 * checkers and events in the application.
 *
 * Users can add different document parsers as well as spell checkers to the
 * core by calling the correct set functions.
 */
class SpellCheckerCore : public QObject
{
    Q_OBJECT
public:
    SpellCheckerCore(QObject *parent = 0);
    ~SpellCheckerCore();

    static SpellCheckerCore* instance();
    /*!
     * \brief Add a new document parser to the Spell Checker.
     * \param parser New parser that can also be used to parse documents.
     * \return False if the parser could not be added. This would normally
     *          happen when the parser was added to the core already.
     */
    bool addDocumentParser(IDocumentParser* parser);
    /*!
     * \brief Remove a document parser previously added to the Spell Checker.
     * \param parser Parser that should be removed.
     */
    void removeDocumentParser(IDocumentParser* parser);

    void addWordsWithSpellingMistakes(const QString& fileName, const WordList& words);

    Internal::OutputPane* outputPane() const;

    ISpellChecker* spellChecker() const;
    QMap<QString, ISpellChecker*> addedSpellCheckers() const;
    void addSpellChecker(ISpellChecker* spellChecker);
    void setSpellChecker(ISpellChecker* spellChecker);

    /*! \brief SpellCheck Words
     *
     * Use the current spellchecker that is set to check the spelling of the given
     * \a words. The full comment that the words appears in is also passed to the
     * spell checker so that it can have some context for the words. This will be
     * needed for abbreviated words. For instance the word 'etc.'. The tokenizer
     * will give the word 'etc' which will fail the spell checker. The spell checker
     * can then check if the word with the full stop added ('etc.') pass the check. This
     * will then also remove some false positives.
     *
     * \param comment Comment that the list of given words belong to
     * \param words List of words that must be checked for spelling mistakes.
     */
    void spellCheckWords(const QString &comment, WordList& words);

    Core::IOptionsPage* optionsPage();

private:
    enum RemoveAction {
        None = 0,
        Ignore,
        Add
    };
    /*!
     * \brief isWordUnderCursorMistake
     * \param[out] word If the word is a mistake, this will return the misspelled word.
     * \return True if the word is misspelled.
     */
    bool isWordUnderCursorMistake(Word& word);
    void removeWordUnderCursor(RemoveAction action);
    
signals:
    void wordUnderCursorMistake(bool isMistake, const Word& word = Word());
    
public slots:
    void startupProjectChanged(ProjectExplorer::Project* startupProject);
    void giveSuggestionsForWordUnderCursor();
    void ignoreWordUnderCursor();
    void addWordUnderCursor();

private slots:
    void cursorPositionChanged();
    void currentEditorChanged(Core::IEditor* editor);
    void editorOpened(Core::IEditor* editor);
    void editorAboutToClose(Core::IEditor* editor);
private:
    Internal::SpellCheckerCorePrivate* const d;
    
};
}

#endif // SPELLCHECKERCORE_H