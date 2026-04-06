#include "mainwindow.h"
#include "treetests.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QSplitter>
#include <QGroupBox>
#include <QMessageBox>
#include <QScrollArea>

// ── Constructor ──────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Binary Tree Visualizer");
    setMinimumSize(1100, 680);

    _buildUi();

    for (int v : {10, 5, 15, 3, 7, 12, 20})
        _tree.add(v);
    _updateTrees();
    _log("Default tree seeded with: 10  5  15  3  7  12  20");
}

// ── UI Construction ──────────────────────────────────────────────────────────

void MainWindow::_buildUi() {

    setStyleSheet(R"(
        QMainWindow  { background: #ecf0f1; }
        QGroupBox {
            font-weight: bold;
            font-size: 11px;
            color: #2c3e50;
            border: 1px solid #bdc3c7;
            border-radius: 6px;
            margin-top: 10px;
            padding: 6px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 4px;
        }
        QLineEdit {
            padding: 6px 8px;
            border: 1px solid #bdc3c7;
            border-radius: 4px;
            background: white;
            color: black;
            font-size: 13px;
        }
        QLineEdit:focus { border-color: #3a7bd5; }
        QTextEdit {
            background: #2c3e50;
            color: #ecf0f1;
            font-family: "Consolas", "Courier New", monospace;
            font-size: 12px;
            border: none;
            border-radius: 4px;
            padding: 6px;
        }
        QPushButton {
            padding: 7px 14px;
            border-radius: 5px;
            font-size: 12px;
            font-weight: bold;
            color: white;
            border: none;
        }
        QLabel { color: #2c3e50; font-size: 12px; }
        QSplitter::handle { background: #bdc3c7; }
    )");

    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *outerLayout = new QVBoxLayout(central);
    outerLayout->setSpacing(8);
    outerLayout->setContentsMargins(12, 12, 12, 12);

    // Main horizontal splitter: left controls | right trees
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, central);
    mainSplitter->setHandleWidth(6);

    // ─── Left control panel ───────────────────────────────────────────────
    QWidget     *leftPanel  = new QWidget;
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(10);
    leftLayout->setContentsMargins(0, 0, 4, 0);

    // Value input
    {
        QGroupBox   *box    = new QGroupBox("Value Input");
        QHBoxLayout *layout = new QHBoxLayout(box);
        QLabel      *lbl    = new QLabel("Node value:");
        _valueInput = new QLineEdit;
        _valueInput->setPlaceholderText("e.g.  42");
        _valueInput->setFixedWidth(110);
        layout->addWidget(lbl);
        layout->addWidget(_valueInput);
        layout->addStretch();
        leftLayout->addWidget(box);
    }

    // Mutators
    {
        QGroupBox   *box  = new QGroupBox("Mutators");
        QGridLayout *grid = new QGridLayout(box);
        grid->setSpacing(6);
        grid->addWidget(_makeButton("Add",            "#27ae60", SLOT(on_addButton_clicked())),           0, 0);
        grid->addWidget(_makeButton("Remove",         "#e74c3c", SLOT(on_removeButton_clicked())),        0, 1);
        grid->addWidget(_makeButton("Remove Subtree", "#c0392b", SLOT(on_removeSubtreeButton_clicked())), 1, 0);
        grid->addWidget(_makeButton("Clear Tree",     "#7f8c8d", SLOT(on_clearButton_clicked())),         1, 1);
        leftLayout->addWidget(box);
    }

    // Queries
    {
        QGroupBox   *box  = new QGroupBox("Queries");
        QGridLayout *grid = new QGridLayout(box);
        grid->setSpacing(6);
        grid->addWidget(_makeButton("Find",       "#3a7bd5", SLOT(on_findButton_clicked())),     0, 0);
        grid->addWidget(_makeButton("Level",      "#3a7bd5", SLOT(on_levelButton_clicked())),    0, 1);
        grid->addWidget(_makeButton("Height",     "#8e44ad", SLOT(on_heightButton_clicked())),   1, 0);
        grid->addWidget(_makeButton("Size",       "#8e44ad", SLOT(on_sizeButton_clicked())),     1, 1);
        grid->addWidget(_makeButton("Min",        "#16a085", SLOT(on_minButton_clicked())),      2, 0);
        grid->addWidget(_makeButton("Max",        "#e67e22", SLOT(on_maxButton_clicked())),      2, 1);
        grid->addWidget(_makeButton("Balanced?",  "#d35400", SLOT(on_balancedButton_clicked())), 3, 0);
        grid->addWidget(_makeButton("Get Vector", "#d35400", SLOT(on_vectorButton_clicked())),   3, 1);
        leftLayout->addWidget(box);
    }

    // Tree 2 / Compare
    {
        QGroupBox   *box  = new QGroupBox("Tree 2 / Compare");
        QVBoxLayout *vbox = new QVBoxLayout(box);
        vbox->setSpacing(6);

        _tree2StatusLabel = new QLabel("Tree 2: empty");
        _tree2StatusLabel->setStyleSheet("color: #7f8c8d; font-style: italic;");
        vbox->addWidget(_tree2StatusLabel);

        QGridLayout *grid = new QGridLayout;
        grid->setSpacing(6);
        grid->addWidget(_makeButton("Duplicate Tree 1", "#2980b9", SLOT(on_duplicateButton_clicked())), 0, 0);
        grid->addWidget(_makeButton("Compare Trees",    "#8e44ad", SLOT(on_compareButton_clicked())),   0, 1);
        grid->addWidget(_makeButton("Clear Tree 2",     "#7f8c8d", SLOT(on_clearTree2Button_clicked())), 1, 0, 1, 2);
        vbox->addLayout(grid);
        leftLayout->addWidget(box);
    }

    // Tests
    {
        QGroupBox   *box    = new QGroupBox("Tests");
        QVBoxLayout *layout = new QVBoxLayout(box);
        auto *btn = _makeButton("Run All Tests", "#1abc9c", SLOT(on_runTestsButton_clicked()));
        layout->addWidget(btn);
        leftLayout->addWidget(box);
    }

    leftLayout->addStretch();
    mainSplitter->addWidget(leftPanel);
    mainSplitter->setStretchFactor(0, 0);

    // ─── Right: two tree panels stacked vertically ────────────────────────
    QSplitter *treeSplitter = new QSplitter(Qt::Vertical);
    treeSplitter->setHandleWidth(6);

    // Tree 1
    {
        QGroupBox   *box    = new QGroupBox("Tree 1  (active)");
        QVBoxLayout *layout = new QVBoxLayout(box);
        _treeWidget1 = new TreeWidget;
        _treeWidget1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        QScrollArea *scroll = new QScrollArea;
        scroll->setWidget(_treeWidget1);
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        layout->addWidget(scroll);
        treeSplitter->addWidget(box);
    }

    // Tree 2
    {
        QGroupBox   *box    = new QGroupBox("Tree 2  (duplicate / compare)");
        QVBoxLayout *layout = new QVBoxLayout(box);
        _treeWidget2 = new TreeWidget;
        _treeWidget2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        QScrollArea *scroll = new QScrollArea;
        scroll->setWidget(_treeWidget2);
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        layout->addWidget(scroll);
        treeSplitter->addWidget(box);
    }

    treeSplitter->setSizes({400, 260});
    mainSplitter->addWidget(treeSplitter);
    mainSplitter->setStretchFactor(1, 1);
    mainSplitter->setSizes({230, 870});

    outerLayout->addWidget(mainSplitter, 1);

    // ─── Output log ───────────────────────────────────────────────────────
    {
        QGroupBox   *box    = new QGroupBox("Output");
        QVBoxLayout *layout = new QVBoxLayout(box);
        layout->setSpacing(4);

        _outputDisplay = new QTextEdit;
        _outputDisplay->setReadOnly(true);
        _outputDisplay->setFixedHeight(150);

        auto *clrBtn = _makeButton("Clear Output", "#95a5a6",
                                   SLOT(on_clearOutputButton_clicked()));
        clrBtn->setFixedWidth(120);

        layout->addWidget(_outputDisplay);
        layout->addWidget(clrBtn, 0, Qt::AlignRight);
        outerLayout->addWidget(box, 0);
    }
}

// ── Button factory ───────────────────────────────────────────────────────────

QPushButton *MainWindow::_makeButton(const QString &label,
                                     const QString &color,
                                     const char    *slot)
{
    auto *btn = new QPushButton(label, this);
    btn->setStyleSheet(
        QString("QPushButton { background-color: %1; }"
                "QPushButton:hover   { background-color: %2; }"
                "QPushButton:pressed { background-color: %3; }")
            .arg(color)
            .arg(QColor(color).lighter(115).name())
            .arg(QColor(color).darker(115).name()));
    connect(btn, SIGNAL(clicked()), this, slot);
    return btn;
}

// ── Helpers ──────────────────────────────────────────────────────────────────

void MainWindow::_updateTrees() {
    _treeWidget1->setTree(&_tree);
    _treeWidget2->setTree(&_tree2);
    _treeWidget1->update();
    _treeWidget2->update();

    bool t2empty = (_tree2.size() == 0);
    _tree2StatusLabel->setText(t2empty
        ? "Tree 2: empty"
        : QString("Tree 2: %1 nodes").arg(_tree2.size()));
    _tree2StatusLabel->setStyleSheet(t2empty
        ? "color: #7f8c8d; font-style: italic;"
        : "color: #2980b9; font-weight: bold;");
}

void MainWindow::_log(const QString &msg) {
    _outputDisplay->append(msg);
}

void MainWindow::_logHtml(const QString &html) {
    _outputDisplay->append(html);
}

bool MainWindow::_getInputValue(int &out) {
    bool ok = false;
    out = _valueInput->text().trimmed().toInt(&ok);
    if (!ok)
        QMessageBox::warning(this, "Invalid Input",
                             "Please enter a valid integer value.");
    return ok;
}

// ── Mutator slots ─────────────────────────────────────────────────────────────

void MainWindow::on_addButton_clicked() {
    int value;
    if (!_getInputValue(value)) return;
    _tree.add(value);
    _treeWidget1->clearHighlight();
    _updateTrees();
    _log(QString("[ADD] Node %1 added.").arg(value));
}

void MainWindow::on_removeButton_clicked() {
    int value;
    if (!_getInputValue(value)) return;
    _treeWidget1->clearHighlight();
    if (_tree.remove(value)) {
        _updateTrees();
        _log(QString("[REMOVE] Node %1 removed.").arg(value));
    } else {
        _log(QString("[REMOVE] Node %1 not found.").arg(value));
    }
}

void MainWindow::on_removeSubtreeButton_clicked() {
    int value;
    if (!_getInputValue(value)) return;
    _treeWidget1->clearHighlight();
    if (!_tree.find(value)) {
        _log(QString("[REMOVE SUBTREE] Node %1 not found.").arg(value));
        return;
    }
    _tree.removeSubtree(value);
    _updateTrees();
    _log(QString("[REMOVE SUBTREE] Subtree rooted at %1 removed.").arg(value));
}

void MainWindow::on_clearButton_clicked() {
    _tree.clear();
    _treeWidget1->clearHighlight();
    _updateTrees();
    _log("[CLEAR] Tree 1 cleared.");
}

// ── Query slots ───────────────────────────────────────────────────────────────

void MainWindow::on_findButton_clicked() {
    int value;
    if (!_getInputValue(value)) return;
    if (_tree.find(value)) {
        _treeWidget1->setHighlight(value, TreeWidget::HighlightMode::Find);
        _log(QString("[FIND] Node %1 found (highlighted in red).").arg(value));
    } else {
        _treeWidget1->clearHighlight();
        _log(QString("[FIND] Node %1 not found.").arg(value));
    }
}

void MainWindow::on_levelButton_clicked() {
    int value;
    if (!_getInputValue(value)) return;
    int lvl = _tree.level(value);
    if (lvl == -1)
        _log(QString("[LEVEL] Node %1 not found.").arg(value));
    else
        _log(QString("[LEVEL] Node %1 is at level %2.").arg(value).arg(lvl));
}

void MainWindow::on_heightButton_clicked() {
    _log(QString("[HEIGHT] Tree height: %1").arg(_tree.height()));
}

void MainWindow::on_sizeButton_clicked() {
    _log(QString("[SIZE] Tree has %1 nodes.").arg(_tree.size()));
}

void MainWindow::on_minButton_clicked() {
    if (_tree.size() == 0) { _log("[MIN] Tree is empty."); return; }
    int m = _tree.min();
    _treeWidget1->setHighlight(m, TreeWidget::HighlightMode::Min);
    _log(QString("[MIN] Minimum value: %1 (highlighted in green).").arg(m));
}

void MainWindow::on_maxButton_clicked() {
    if (_tree.size() == 0) { _log("[MAX] Tree is empty."); return; }
    int m = _tree.max();
    _treeWidget1->setHighlight(m, TreeWidget::HighlightMode::Max);
    _log(QString("[MAX] Maximum value: %1 (highlighted in orange).").arg(m));
}

void MainWindow::on_balancedButton_clicked() {
    if (_tree.balanced())
        _log("[BALANCED] The tree IS balanced.");
    else
        _log("[BALANCED] The tree is NOT balanced.");
}

void MainWindow::on_vectorButton_clicked() {
    auto vec = _tree.getVector();
    QString out = "[VECTOR] Sorted: [ ";
    for (int v : vec) out += QString::number(v) + "  ";
    out += "]";
    _log(out);
}

// ── Tree 2 / Compare slots ────────────────────────────────────────────────────

void MainWindow::on_duplicateButton_clicked() {
    _tree2 = _tree;                      // uses copy-assignment operator
    _treeWidget2->clearHighlight();
    _updateTrees();
    _log(QString("[DUPLICATE] Tree 1 duplicated into Tree 2 (%1 nodes).")
             .arg(_tree2.size()));
}

void MainWindow::on_compareButton_clicked() {
    if (_tree == _tree2)
        _logHtml("<span style='color:#27ae60;'>[COMPARE] Trees are EQUAL.</span>");
    else
        _logHtml("<span style='color:#e74c3c;'>[COMPARE] Trees are NOT equal.</span>");
}

void MainWindow::on_clearTree2Button_clicked() {
    _tree2.clear();
    _treeWidget2->clearHighlight();
    _updateTrees();
    _log("[CLEAR] Tree 2 cleared.");
}

// ── Test runner slot ──────────────────────────────────────────────────────────

void MainWindow::on_runTestsButton_clicked() {
    _outputDisplay->clear();
    _logHtml("<b style='color:#1abc9c;'>====== Running Tests ======</b>");

    QVector<TestResult> results = TreeTests::runAll();

    int passed = 0, failed = 0;
    QString currentSection;

    // Section boundaries (match order in TreeTests::runAll)
    QStringList sections = {
        "Basic Operations",
        "Search and Stats",
        "Structure and Balancing",
        "BFS Iterators",
        "Vector and Operators",
        "Random Tree"
    };
    int secIdx = 0;
    int sectionCounts[] = {4, 6, 2, 2, 6, 5}; // tests per section
    int secCount = 0;

    for (const TestResult &r : results) {
        // Section header
        if (secCount == 0 && secIdx < sections.size()) {
            _logHtml(QString("<b style='color:#f39c12;'>--- %1 ---</b>")
                         .arg(sections[secIdx]));
        }

        // Result line
        if (r.passed) {
            ++passed;
            QString line = QString("<span style='color:#27ae60;'>[PASS]</span> %1")
                               .arg(r.name.toHtmlEscaped());
            if (!r.detail.isEmpty())
                line += QString(" <span style='color:#7f8c8d;'>(%1)</span>")
                            .arg(r.detail.toHtmlEscaped());
            _logHtml(line);
        } else {
            ++failed;
            QString line = QString("<span style='color:#e74c3c;'>[FAIL]</span> %1")
                               .arg(r.name.toHtmlEscaped());
            if (!r.detail.isEmpty())
                line += QString(" <span style='color:#e74c3c;'>-- %1</span>")
                            .arg(r.detail.toHtmlEscaped());
            _logHtml(line);
        }

        ++secCount;
        if (secIdx < sections.size() && secCount >= sectionCounts[secIdx]) {
            secCount = 0;
            ++secIdx;
        }
    }

    // Summary
    _logHtml(QString("<b>Results: "
                     "<span style='color:#27ae60;'>%1 passed</span> / "
                     "<span style='color:#e74c3c;'>%2 failed</span> / "
                     "%3 total</b>")
                 .arg(passed).arg(failed).arg(results.size()));
}

// ── Utility ──────────────────────────────────────────────────────────────────

void MainWindow::on_clearOutputButton_clicked() {
    _outputDisplay->clear();
}
