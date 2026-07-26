#include <QtTest>
#include <QFileDialog>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickTextDocument>
#include <QQuickWindow>
#include <QQuickStyle>
#include <QTextBlock>

#include "backend.h"
#include "markdownhighlighter.h"

class OmawriteTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        QVERIFY(m_settingsDirectory.isValid());
        QQuickStyle::setStyle(QStringLiteral("Material"));
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                           m_settingsDirectory.path());
    }

    void countsWords() {
        QCOMPARE(Backend::countWords(QStringLiteral("one two-three don't 42")), 4);
        QCOMPARE(Backend::countWords(QStringLiteral("你好 世界")), 2);
        QCOMPARE(Backend::countWords(QString()), 0);
    }

    void normalizesLinks() {
        QCOMPARE(Backend::normalizedLinkUrl(QStringLiteral("www.example.com/path")),
                 QStringLiteral("https://www.example.com/path"));
        QCOMPARE(Backend::normalizedLinkUrl(QStringLiteral("mailto:writer@example.com")),
                 QStringLiteral("mailto:writer@example.com"));
        QVERIFY(Backend::normalizedLinkUrl(QStringLiteral("example.com")).isEmpty());
        QVERIFY(Backend::normalizedLinkUrl(QStringLiteral("file:///tmp/private")).isEmpty());
    }

    void suggestsSafeNames() {
        QCOMPARE(Backend::suggestedFileName(QStringLiteral("My first draft\nBody")),
                 QStringLiteral("My first draft.md"));
        QCOMPARE(Backend::suggestedFileName(QStringLiteral("A/B")), QStringLiteral("A-B.md"));
        QCOMPARE(Backend::suggestedFileName(QString()), QStringLiteral("Untitled.md"));
        QCOMPARE(Backend::suggestedFileName(QStringLiteral("Already.md")),
                 QStringLiteral("Already.md"));
    }

    void findsInlineMarkdownRanges() {
        const auto markup = MarkdownHighlighter::inlineMarkup(
            QStringLiteral("**bold** and *italic* and [site](https://example.com)"));
        QCOMPARE(markup.size(), 3);
        QCOMPARE(markup.at(0).content.start, 2);
        QCOMPARE(markup.at(0).content.length, 4);
        QCOMPARE(markup.at(2).content.length, 4);
        QCOMPARE(markup.at(2).markers[0].length, 1);
    }

    void loadsCurrentOmarchyTheme() {
        QTemporaryDir homeDirectory;
        QVERIFY(homeDirectory.isValid());

        const QByteArray originalHome = qgetenv("HOME");
        struct HomeRestorer {
            QByteArray value;
            ~HomeRestorer() { qputenv("HOME", value); }
        } restoreHome{originalHome};
        QVERIFY(qputenv("HOME", homeDirectory.path().toUtf8()));

        const QString themeDirectory = homeDirectory.path()
            + QStringLiteral("/.local/state/omarchy/current/theme");
        QVERIFY(QDir().mkpath(themeDirectory));

        QFile colorsFile(themeDirectory + QStringLiteral("/colors.toml"));
        QVERIFY(colorsFile.open(QIODevice::WriteOnly | QIODevice::Text));
        const QByteArray palette(
            "mode = \"light\"\n"
            "accent = \"#112233\"\n"
            "selection = \"#445566\"\n"
            "background = \"#fefefe\"\n"
            "foreground = \"#101010\"\n");
        QCOMPARE(colorsFile.write(palette), qint64(palette.size()));
        colorsFile.close();

        Backend backend;
        QCOMPARE(backend.themeBackground(), QStringLiteral("#fefefe"));
        QCOMPARE(backend.themeForeground(), QStringLiteral("#101010"));
        QCOMPARE(backend.themeAccent(), QStringLiteral("#112233"));
        QCOMPARE(backend.themeSelection(), QStringLiteral("#445566"));
        QVERIFY(!backend.darkMode());
    }

    void ignoresFileWatcherEventsForSavedContents() {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString path = directory.filePath(QStringLiteral("first-save.md"));
        Backend backend;
        QSignalSpy externalChangeSpy(&backend, &Backend::externalChangeDetected);

        backend.saveAs(QUrl::fromLocalFile(path));
        QVERIFY(QFileInfo::exists(path));

        QFile sameContents(path);
        QVERIFY(sameContents.open(QIODevice::WriteOnly | QIODevice::Truncate));
        sameContents.close();
        QTest::qWait(100);
        QCOMPARE(externalChangeSpy.count(), 0);

        QFile changedContents(path);
        QVERIFY(changedContents.open(QIODevice::WriteOnly | QIODevice::Truncate));
        QCOMPARE(changedContents.write("changed elsewhere"), qint64(17));
        changedContents.close();
        QTRY_COMPARE(externalChangeSpy.count(), 1);
    }

    void recognizesExistingLocalFiles() {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString existingPath = directory.filePath(QStringLiteral("existing.md"));
        QFile existingFile(existingPath);
        QVERIFY(existingFile.open(QIODevice::WriteOnly));
        existingFile.close();

        Backend backend;
        bool exists = false;
        QVERIFY(QMetaObject::invokeMethod(
            &backend, "fileExists", Q_RETURN_ARG(bool, exists),
            Q_ARG(QUrl, QUrl::fromLocalFile(existingPath))));
        QVERIFY(exists);

        QVERIFY(QMetaObject::invokeMethod(
            &backend, "fileExists", Q_RETURN_ARG(bool, exists),
            Q_ARG(QUrl, QUrl::fromLocalFile(directory.filePath(QStringLiteral("new.md"))))));
        QVERIFY(!exists);
    }

    void keepsCursorAndSelectionStableAcrossInsertions() {
        const QString mutationsPath = QFINDTESTDATA("../src/EditorMutations.js");
        QVERIFY(!mutationsPath.isEmpty());

        QQmlEngine engine;
        QQmlComponent component(&engine);
        const QByteArray harness = R"QML(
            import QtQuick
            import "EditorMutations.js" as EditorMutations

            TextEdit {
                property string insertionText
                property int insertionCursor
                property string wrappedText
                property int wrappedSelectionStart
                property int wrappedSelectionEnd

                Component.onCompleted: {
                    text = "alpha omega";
                    cursorPosition = 5;
                    EditorMutations.replaceRange(this, 5, 5, "one\r\ntwo");
                    insertionText = text;
                    insertionCursor = cursorPosition;

                    text = "alpha beta omega";
                    select(6, 10);
                    EditorMutations.replaceRange(this, selectionStart, selectionEnd,
                                                 "**beta**", 2, 6);
                    wrappedText = text;
                    wrappedSelectionStart = selectionStart;
                    wrappedSelectionEnd = selectionEnd;
                }
            }
        )QML";
        const QUrl harnessUrl = QUrl::fromLocalFile(
            QFileInfo(mutationsPath).absolutePath() + QStringLiteral("/MutationHarness.qml"));
        component.setData(harness, harnessUrl);
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        QScopedPointer<QObject> editor(component.create());
        QVERIFY2(editor, qPrintable(component.errorString()));

        QCOMPARE(editor->property("insertionText").toString(),
                 QStringLiteral("alphaone\ntwo omega"));
        QCOMPARE(editor->property("insertionCursor").toInt(), 12);
        QCOMPARE(editor->property("wrappedText").toString(),
                 QStringLiteral("alpha **beta** omega"));
        QCOMPARE(editor->property("wrappedSelectionStart").toInt(), 8);
        QCOMPARE(editor->property("wrappedSelectionEnd").toInt(), 12);
    }

    void savesAndOpensFromFooterButtons() {
        const QString mainQmlPath = QFINDTESTDATA("../src/Main.qml");
        QVERIFY(!mainQmlPath.isEmpty());

        Backend backend;
        QQmlEngine engine;
        engine.rootContext()->setContextProperty(QStringLiteral("backend"), &backend);
        QQmlComponent component(&engine, QUrl::fromLocalFile(mainQmlPath));
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        QScopedPointer<QObject> window(component.create());
        QVERIFY2(window, qPrintable(component.errorString()));

        QVERIFY(window->findChild<QObject *>(QStringLiteral("sourceEditor")));
        QVERIFY(!window->findChild<QObject *>(QStringLiteral("renderedPreview")));
        QVERIFY(!window->findChild<QObject *>(QStringLiteral("modeToggle")));

        QObject *saveButton = window->findChild<QObject *>(QStringLiteral("saveButton"));
        QObject *saveAsButton =
            window->findChild<QObject *>(QStringLiteral("saveAsButton"));
        QObject *openButton = window->findChild<QObject *>(QStringLiteral("openButton"));
        QVERIFY(saveButton);
        QVERIFY(saveAsButton);
        QVERIFY(openButton);
        QVERIFY(saveAsButton->property("x").toReal() < saveButton->property("x").toReal());
        QVERIFY(saveButton->property("x").toReal() < openButton->property("x").toReal());

        QSignalSpy saveDialogSpy(&backend, &Backend::saveDialogRequested);
        QVERIFY(QMetaObject::invokeMethod(saveAsButton, "clicked"));
        QCOMPARE(saveDialogSpy.count(), 1);

        QVERIFY(QMetaObject::invokeMethod(saveButton, "clicked"));
        QCOMPARE(saveDialogSpy.count(), 2);

        QSignalSpy openDialogSpy(&backend, &Backend::openDialogRequested);
        QVERIFY(QMetaObject::invokeMethod(openButton, "clicked"));
        QCOMPARE(openDialogSpy.count(), 1);
    }

    void confirmsOverwriteInsideOmawrite() {
        const QString mainQmlPath = QFINDTESTDATA("../src/Main.qml");
        QVERIFY(!mainQmlPath.isEmpty());
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString existingPath = directory.filePath(QStringLiteral("existing.md"));
        QFile existingFile(existingPath);
        QVERIFY(existingFile.open(QIODevice::WriteOnly));
        existingFile.close();

        Backend backend;
        QQmlEngine engine;
        engine.rootContext()->setContextProperty(QStringLiteral("backend"), &backend);
        QQmlComponent component(&engine, QUrl::fromLocalFile(mainQmlPath));
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        QScopedPointer<QObject> root(component.create());
        QVERIFY2(root, qPrintable(component.errorString()));

        QObject *saveFileDialog =
            root->findChild<QObject *>(QStringLiteral("saveFileDialog"));
        QObject *overwriteDialog =
            root->findChild<QObject *>(QStringLiteral("overwriteDialog"));
        QVERIFY(saveFileDialog);
        QVERIFY(overwriteDialog);
        QVERIFY(saveFileDialog->property("options").toInt()
                & int(QFileDialog::DontConfirmOverwrite));

        const QUrl existingUrl = QUrl::fromLocalFile(existingPath);
        QVERIFY(QMetaObject::invokeMethod(
            root.data(), "requestSaveAs", Q_ARG(QVariant, QVariant(existingUrl))));
        QTRY_VERIFY(overwriteDialog->property("opened").toBool());
        QCOMPARE(overwriteDialog->property("fileName").toString(),
                 QStringLiteral("existing.md"));
        QCOMPARE(root->property("keybindingsOverlayColor").value<QColor>(),
                 QColor(QStringLiteral("#99000000")));

        QObject *cancelButton =
            overwriteDialog->findChild<QObject *>(QStringLiteral("overwriteCancelButton"));
        QObject *noButton =
            overwriteDialog->findChild<QObject *>(QStringLiteral("overwriteNoButton"));
        QObject *confirmButton =
            overwriteDialog->findChild<QObject *>(QStringLiteral("overwriteConfirmButton"));
        QVERIFY(cancelButton);
        QVERIFY(noButton);
        QVERIFY(confirmButton);
        QVERIFY(cancelButton->property("x").toReal() < noButton->property("x").toReal());
        QVERIFY(noButton->property("x").toReal() < confirmButton->property("x").toReal());

        QVERIFY(QMetaObject::invokeMethod(noButton, "clicked"));
        QTRY_VERIFY(!overwriteDialog->property("opened").toBool());
        QTRY_VERIFY(saveFileDialog->property("visible").toBool());
    }

    void keyboardShortcutsActivate() {
        const QString mainQmlPath = QFINDTESTDATA("../src/Main.qml");
        QVERIFY(!mainQmlPath.isEmpty());

        Backend backend;
        QQmlEngine engine;
        engine.rootContext()->setContextProperty(QStringLiteral("backend"), &backend);
        QQmlComponent component(&engine, QUrl::fromLocalFile(mainQmlPath));
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        QScopedPointer<QObject> root(component.create());
        QVERIFY2(root, qPrintable(component.errorString()));

        auto *window = qobject_cast<QQuickWindow *>(root.data());
        QVERIFY(window);
        window->show();
        window->requestActivate();
        QTRY_VERIFY(window->isActive());

        QObject *unsavedDialog =
            window->findChild<QObject *>(QStringLiteral("unsavedChangesDialog"));
        QVERIFY(unsavedDialog);
        QCOMPARE(unsavedDialog->property("overlayColor").value<QColor>(),
                 QColor(QStringLiteral("#99000000")));

        QSignalSpy openDialogSpy(&backend, &Backend::openDialogRequested);
        QTest::keyClick(window, Qt::Key_O, Qt::ControlModifier);
        QTRY_COMPARE(openDialogSpy.count(), 1);

        QVERIFY(!window->property("searchOpen").toBool());
        QTest::keyClick(window, Qt::Key_F, Qt::ControlModifier);
        QTRY_VERIFY(window->property("searchOpen").toBool());

        QObject *shortcutsDialog =
            window->findChild<QObject *>(QStringLiteral("shortcutsDialog"));
        QVERIFY(shortcutsDialog);
        QVERIFY(!shortcutsDialog->property("opened").toBool());

        QTest::keyClick(window, Qt::Key_F1);
        QTRY_VERIFY(shortcutsDialog->property("opened").toBool());
        QObject *shortcutReference =
            shortcutsDialog->findChild<QObject *>(QStringLiteral("shortcutReference"));
        QVERIFY(shortcutReference);
        const QString shortcutText = shortcutReference->property("text").toString();
        QVERIFY(!shortcutText.contains(QStringLiteral("Ctrl+?")));
        QVERIFY(!shortcutText.contains(QStringLiteral("Ctrl+Y")));
        QVERIFY(!shortcutText.contains(QStringLiteral("Super+F")));
        QCOMPARE(window->property("keybindingsOverlayColor").value<QColor>(),
                 QColor(QStringLiteral("#99000000")));
    }

    void footerKeybindingsHintAdapts() {
        const QString mainQmlPath = QFINDTESTDATA("../src/Main.qml");
        QVERIFY(!mainQmlPath.isEmpty());

        Backend backend;
        QQmlEngine engine;
        engine.rootContext()->setContextProperty(QStringLiteral("backend"), &backend);
        QQmlComponent component(&engine, QUrl::fromLocalFile(mainQmlPath));
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        QScopedPointer<QObject> root(component.create());
        QVERIFY2(root, qPrintable(component.errorString()));

        auto *window = qobject_cast<QQuickWindow *>(root.data());
        QVERIFY(window);
        QObject *status =
            window->findChild<QObject *>(QStringLiteral("footerDocumentStatus"));
        QObject *hint =
            window->findChild<QObject *>(QStringLiteral("footerKeybindingsHint"));
        QObject *wordCount =
            window->findChild<QObject *>(QStringLiteral("footerWordCount"));
        QObject *editor =
            window->findChild<QObject *>(QStringLiteral("sourceEditor"));
        QVERIFY(status);
        QVERIFY(hint);
        QVERIFY(wordCount);
        QVERIFY(editor);

        const QColor footerColor(QStringLiteral("#e0af68"));
        QCOMPARE(status->property("color").value<QColor>(), footerColor);
        QCOMPARE(status->parent()->property("opacity").toReal(), qreal(0.95));
        QCOMPARE(hint->property("text").toString(), QStringLiteral("F1: Keybindings"));
        QCOMPARE(hint->property("color").value<QColor>(), footerColor);
        QCOMPARE(hint->property("opacity").toReal(), qreal(0.95));
        QCOMPARE(wordCount->property("color").value<QColor>(), footerColor);
        QCOMPARE(wordCount->property("opacity").toReal(), qreal(0.95));
        QCOMPARE(window->opacity(), qreal(0.85));

        editor->setProperty("text", QStringLiteral("draft"));
        QTRY_VERIFY(status->property("visible").toBool());

        window->setWidth(1280);
        QTRY_VERIFY(hint->property("visible").toBool());
        const qreal hintCenter = hint->property("x").toReal()
            + hint->property("width").toReal() / 2.0;
        QVERIFY(qAbs(hintCenter - window->width() / 2.0) <= 1.0);

        window->setWidth(720);
        QTRY_VERIFY(!hint->property("visible").toBool());
        window->setWidth(1280);
        QTRY_VERIFY(hint->property("visible").toBool());
    }

    void usesConfiguredTypography() {
        const QString mainQmlPath = QFINDTESTDATA("../src/Main.qml");
        QVERIFY(!mainQmlPath.isEmpty());

        Backend backend;
        QQmlEngine engine;
        engine.rootContext()->setContextProperty(QStringLiteral("backend"), &backend);
        QQmlComponent component(&engine, QUrl::fromLocalFile(mainQmlPath));
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        QScopedPointer<QObject> root(component.create());
        QVERIFY2(root, qPrintable(component.errorString()));

        QObject *editor =
            root->findChild<QObject *>(QStringLiteral("sourceEditor"));
        QObject *footer =
            root->findChild<QObject *>(QStringLiteral("footerWordCount"));
        QVERIFY(editor);
        QVERIFY(footer);
        QCOMPARE(editor->property("font").value<QFont>().family(),
                 QStringLiteral("iA Writer Mono S"));
        QCOMPARE(editor->property("font").value<QFont>().pixelSize(), 18);
        QCOMPARE(footer->property("font").value<QFont>().pixelSize(), 12);

        auto *quickDocument =
            editor->property("textDocument").value<QQuickTextDocument *>();
        QVERIFY(quickDocument);
        QVERIFY(quickDocument->textDocument());
        QCOMPARE(quickDocument->textDocument()->firstBlock().blockFormat().lineHeight(),
                 qreal(120));
    }

    void remembersLastSaveDirectory() {
        QTemporaryDir saveDirectory;
        QVERIFY(saveDirectory.isValid());

        const QString savedPath = saveDirectory.filePath(QStringLiteral("first.md"));
        Backend savedDocument;
        savedDocument.saveAs(QUrl::fromLocalFile(savedPath));

        Backend nextDocument;
        QSignalSpy saveDialogSpy(&nextDocument, &Backend::saveDialogRequested);
        nextDocument.saveAsDialog();
        QCOMPARE(saveDialogSpy.count(), 1);

        const QUrl suggestedUrl = saveDialogSpy.takeFirst().constFirst().toUrl();
        QCOMPARE(QFileInfo(suggestedUrl.toLocalFile()).absolutePath(),
                 saveDirectory.path());
        QCOMPARE(QFileInfo(suggestedUrl.toLocalFile()).fileName(),
                 QStringLiteral("Untitled.md"));

        QSettings().setValue(QStringLiteral("file/lastSaveDirectory"),
                             saveDirectory.filePath(QStringLiteral("missing")));
        Backend fallbackDocument;
        QSignalSpy fallbackDialogSpy(&fallbackDocument, &Backend::saveDialogRequested);
        fallbackDocument.saveAsDialog();
        const QUrl fallbackUrl = fallbackDialogSpy.takeFirst().constFirst().toUrl();
        QCOMPARE(QFileInfo(fallbackUrl.toLocalFile()).absolutePath(), QDir::homePath());
    }

private:
    QTemporaryDir m_settingsDirectory;
};

QTEST_MAIN(OmawriteTest)
#include "tst_omawrite.moc"
