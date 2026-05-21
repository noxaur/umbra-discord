#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QTextCursor>
#include <QStatusBar>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QSplitter>
#include <QFrame>
#include <QDebug>
#include <QSettings>
#include <QToolButton>
#include <QGroupBox>
#include <QTabWidget>
#include "core/client.h"
#include "core/formatting/discord_markdown.h"
#include "core/formatting/embed_renderer.h"
#include "core/formatting/attachment_renderer.h"
#include "core/ui/theme.h"
#include "core/utils.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr)
        : QMainWindow(parent)
    {
        setWindowTitle("Umbra");
        resize(1000, 650);
        setMinimumSize(700, 450);

        // Central splitter: server list | channel list | messages
        auto *splitter = new QSplitter(Qt::Horizontal, this);
        setCentralWidget(splitter);

        // Load saved token
        QSettings settings("Umbra", "discord-qt");
        m_savedToken = settings.value("bot_token").toString();

        // --- Server list (left sidebar) ---
        auto *serverFrame = new QFrame(splitter);
        serverFrame->setFrameShape(QFrame::NoFrame);
        auto *serverLayout = new QVBoxLayout(serverFrame);
        serverLayout->setContentsMargins(0, 0, 0, 0);
        serverLayout->setSpacing(0);

        m_serverList = new QListWidget(serverFrame);
        m_serverList->setObjectName("serverList");
        m_serverList->setMaximumWidth(200);
        m_serverList->setMinimumWidth(60);
        m_serverList->setIconSize(QSize(32, 32));
        serverLayout->addWidget(m_serverList);

        splitter->addWidget(serverFrame);

        // --- Channel list (middle sidebar) ---
        m_channelFrame = new QFrame(splitter);
        m_channelFrame->setObjectName("channelFrame");
        m_channelFrame->setFrameShape(QFrame::NoFrame);
        auto *channelLayout = new QVBoxLayout(m_channelFrame);
        channelLayout->setContentsMargins(0, 0, 0, 0);
        channelLayout->setSpacing(0);

        m_channelLabel = new QLabel("Select a server", m_channelFrame);
        m_channelLabel->setObjectName("channelHeader");
        channelLayout->addWidget(m_channelLabel);

        m_channelTree = new QTreeWidget(m_channelFrame);
        m_channelTree->setObjectName("channelTree");
        m_channelTree->setHeaderHidden(true);
        m_channelTree->setIndentation(16);
        m_channelTree->setAnimated(true);
        m_channelTree->setRootIsDecorated(false);
        m_channelTree->setSelectionMode(QAbstractItemView::SingleSelection);
        channelLayout->addWidget(m_channelTree);

        splitter->addWidget(m_channelFrame);

        // --- Main content (right) ---
        auto *mainWidget = new QWidget(splitter);
        auto *mainLayout = new QVBoxLayout(mainWidget);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        // Top bar with settings button
        auto *topBar = new QHBoxLayout();
        topBar->setContentsMargins(12, 8, 8, 8);
        topBar->addStretch();

        m_settingsBtn = new QToolButton(mainWidget);
        m_settingsBtn->setObjectName("settingsBtn");
        m_settingsBtn->setText("Options");
        m_settingsBtn->setToolTip("Settings");
        m_settingsBtn->setCursor(Qt::PointingHandCursor);
        m_settingsBtn->setFixedSize(64, 32);
        topBar->addWidget(m_settingsBtn);
        mainLayout->addLayout(topBar);

        // Login bar (hidden when logged in)
        m_loginBarWidget = new QWidget(mainWidget);
        auto *loginBar = new QHBoxLayout(m_loginBarWidget);
        loginBar->setContentsMargins(12, 0, 12, 10);
        loginBar->setSpacing(8);

        m_tokenInput = new QLineEdit(m_loginBarWidget);
        m_tokenInput->setObjectName("tokenInput");
        m_tokenInput->setEchoMode(QLineEdit::Password);
        m_tokenInput->setPlaceholderText("Bot token");
        if (!m_savedToken.isEmpty()) {
            m_tokenInput->setText(m_savedToken);
        }
        loginBar->addWidget(m_tokenInput, 1);

        m_loginBtn = new QPushButton("Connect", m_loginBarWidget);
        loginBar->addWidget(m_loginBtn);
        mainLayout->addWidget(m_loginBarWidget);

        // Channel name header
        auto *headerBar = new QHBoxLayout();
        headerBar->setContentsMargins(12, 8, 12, 8);
        m_channelNameLabel = new QLabel("", mainWidget);
        m_channelNameLabel->setObjectName("channelNameLabel");
        m_channelNameLabel->setVisible(false);
        headerBar->addWidget(m_channelNameLabel);
        headerBar->addStretch();
        mainLayout->addLayout(headerBar);

        // Separator line
        auto *sep = new QFrame(mainWidget);
        sep->setObjectName("separatorLine");
        sep->setFrameShape(QFrame::NoFrame);
        sep->setFixedHeight(1);
        sep->setMinimumHeight(1);
        sep->setMaximumHeight(1);
        mainLayout->addWidget(sep);

        auto *loadMoreLayout = new QHBoxLayout();
        loadMoreLayout->setContentsMargins(12, 8, 12, 4);
        loadMoreLayout->addStretch();
        m_loadMoreBtn = new QPushButton("Load older messages", mainWidget);
        m_loadMoreBtn->setObjectName("loadMoreBtn");
        m_loadMoreBtn->setVisible(false);
        m_loadMoreBtn->setCursor(Qt::PointingHandCursor);
        m_loadMoreBtn->setFixedHeight(32);
        loadMoreLayout->addWidget(m_loadMoreBtn);
        loadMoreLayout->addStretch();
        mainLayout->addLayout(loadMoreLayout);

        // Message view
        m_messageView = new QTextEdit(mainWidget);
        m_messageView->setObjectName("messageView");
        m_messageView->setReadOnly(true);
        mainLayout->addWidget(m_messageView);

        // Message input
        auto *inputLayout = new QHBoxLayout();
        inputLayout->setContentsMargins(12, 8, 12, 12);
        inputLayout->setSpacing(8);

        m_messageInput = new QLineEdit(mainWidget);
        m_messageInput->setObjectName("messageInput");
        m_messageInput->setPlaceholderText("Message");
        m_messageInput->setEnabled(false);
        inputLayout->addWidget(m_messageInput, 1);

        m_sendBtn = new QPushButton("Send", mainWidget);
        m_sendBtn->setEnabled(false);
        inputLayout->addWidget(m_sendBtn);
        mainLayout->addLayout(inputLayout);

        splitter->addWidget(mainWidget);

        // Splitter proportions
        splitter->setSizes({80, 200, 720});
        splitter->setStretchFactor(2, 1);

        // Menu bar
        auto *menuBar = this->menuBar();
        auto *helpMenu = menuBar->addMenu("&Help");
        auto *aboutAction = helpMenu->addAction("&About");
        connect(aboutAction, &QAction::triggered, this, [this]() {
            QMessageBox::about(this, "About Umbra",
                "Umbra v0.1.0\n\n"
                "A Discord bot client built with Qt6 and C++.\n"
                "github.com/noxaur/umbra-discord");
        });

        // Client setup
        m_client.setIntents(Intent::Guilds | Intent::GuildMessages | Intent::DirectMessages);

        // Inject message CSS into QTextEdit document
        m_messageView->document()->setDefaultStyleSheet(ThemeManager::instance().regenerateMessageCSS());

        // Update message CSS when theme changes
        connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](const Theme &) {
            m_messageView->document()->setDefaultStyleSheet(ThemeManager::instance().regenerateMessageCSS());
        });

        // Signal connections
        connect(m_loginBtn, &QPushButton::clicked, this, &MainWindow::onLoginClicked);
        connect(m_sendBtn, &QPushButton::clicked, this, &MainWindow::onSendClicked);
        connect(m_messageInput, &QLineEdit::returnPressed, this, &MainWindow::onSendClicked);
        connect(m_settingsBtn, &QToolButton::clicked, this, &MainWindow::onSettingsClicked);
        connect(m_loadMoreBtn, &QPushButton::clicked, this, &MainWindow::onLoadMoreClicked);

        connect(m_serverList, &QListWidget::currentRowChanged, this, &MainWindow::onServerSelected);
        connect(m_channelTree, &QTreeWidget::currentItemChanged, this, &MainWindow::onChannelTreeChanged);

        connect(&m_client, &Client::ready, this, &MainWindow::onReady);
        connect(&m_client, &Client::guildAdded, this, &MainWindow::onGuildAdded);
        connect(&m_client, &Client::channelAdded, this, &MainWindow::onChannelAdded);
        connect(&m_client, &Client::messagesLoaded, this, &MainWindow::onMessagesLoaded);
        connect(&m_client, &Client::messageReceived, this, &MainWindow::onMessageReceived);
        connect(m_client.rest(), &RestClient::messageSent, this, &MainWindow::onMessageSent);
        connect(&m_client, &Client::connectionStateChanged, this, &MainWindow::onConnectionChanged);
        connect(&m_client, &Client::error, this, &MainWindow::onError);

        // Status bar
        setStatusBar(new QStatusBar(this));
        statusBar()->showMessage("Not connected");

        // Auto-connect if token was saved
        if (!m_savedToken.isEmpty()) {
            onLoginClicked();
        }
    }

private slots:
    void onGuildAdded(const Guild &guild)
    {
        for (int i = 0; i < m_serverList->count(); ++i) {
            auto *item = m_serverList->item(i);
            if (item->data(Qt::UserRole).toString() == guild.id.toString()) {
                return;
            }
        }
        auto *item = new QListWidgetItem(guild.name, m_serverList);
        item->setData(Qt::UserRole, guild.id.toString());
        item->setToolTip(guild.name);

        if (m_serverList->count() == 1) {
            m_serverList->setCurrentRow(0);
        }
    }

    void onChannelAdded(const Channel &channel)
    {
        addChannelToTree(channel);
    }

    void addChannelToTree(const Channel &channel)
    {
        if (channel.type == ChannelType::GuildCategory) {
            auto *catItem = new QTreeWidgetItem(m_channelTree);
            catItem->setText(0, "▾ " + channel.name.toUpper());
            catItem->setData(0, Qt::UserRole, channel.id.toString());
            catItem->setData(0, Qt::UserRole + 1, "category");
            catItem->setExpanded(true);
            m_categoryItems[channel.id.toString()] = catItem;
            return;
        }

        if (channel.type == ChannelType::Text || channel.type == ChannelType::DM ||
            channel.type == ChannelType::GuildAnnouncement) {
            QTreeWidgetItem *parent = nullptr;
            if (channel.parentId.has_value()) {
                QString parentIdStr = channel.parentId->toString();
                if (m_categoryItems.contains(parentIdStr)) {
                    parent = m_categoryItems[parentIdStr];
                }
            }

            auto *chItem = new QTreeWidgetItem(parent);
            chItem->setText(0, "# " + channel.name);
            chItem->setData(0, Qt::UserRole, channel.id.toString());
            chItem->setData(0, Qt::UserRole + 1, "channel");
            chItem->setToolTip(0, channel.name);
            return;
        }

        if (channel.type == ChannelType::Voice || channel.type == ChannelType::GuildStageVoice) {
            QTreeWidgetItem *parent = nullptr;
            if (channel.parentId.has_value()) {
                QString parentIdStr = channel.parentId->toString();
                if (m_categoryItems.contains(parentIdStr)) {
                    parent = m_categoryItems[parentIdStr];
                }
            }

            auto *chItem = new QTreeWidgetItem(parent);
            chItem->setText(0, QString::fromUtf8("\u25B6 ") + channel.name);
            chItem->setData(0, Qt::UserRole, channel.id.toString());
            chItem->setData(0, Qt::UserRole + 1, "voice");
            chItem->setForeground(0, QBrush(QColor(ThemeManager::instance().currentTheme().mute)));
            return;
        }
    }

    void onMessagesLoaded(const QList<Message> &messages)
    {
        bool isPagination = m_isLoadingOlderMessages;
        m_isLoadingOlderMessages = false;

        if (!isPagination) {
            m_messageView->clear();
            m_lastAuthorId.clear();
            m_messageCount = 0;
        }

        if (messages.isEmpty() && !isPagination) {
            m_messageView->append(
                "<div class=\"empty-state\">"
                "<div class=\"empty-state-title\">No messages yet</div>"
                "<div class=\"empty-state-body\">Start the conversation in this channel.</div>"
                "</div>"
            );
            m_hasMoreMessages = false;
            m_loadMoreBtn->setVisible(false);
            return;
        }

        for (const auto &msg : messages) {
            appendMessage(msg);
        }

        if (!isPagination) {
            m_messageView->moveCursor(QTextCursor::End);
        }

        if (messages.size() < 50) {
            m_hasMoreMessages = false;
        } else {
            m_hasMoreMessages = true;
        }

        if (!messages.isEmpty()) {
            m_oldestMessageId = messages.last().id;
        }

        m_loadMoreBtn->setVisible(m_hasMoreMessages);
        m_loadMoreBtn->setEnabled(true);
        m_loadMoreBtn->setText("Load older messages");
    }

    void onLoginClicked()
    {
        QString token = m_tokenInput->text().trimmed();
        if (token.isEmpty()) {
            QMessageBox::warning(this, "Error", "Please enter a bot token.");
            return;
        }

        // Save token
        m_savedToken = token;
        QSettings settings("Umbra", "discord-qt");
        settings.setValue("bot_token", token);

        // Hide login bar
        m_loginBarWidget->setVisible(false);

        m_loginBtn->setEnabled(false);
        m_tokenInput->setEnabled(false);
        statusBar()->showMessage("Connecting...");
        m_client.login(token);
    }

    void onSettingsClicked()
    {
        auto *dialog = new QDialog(this);
        dialog->setWindowTitle("Settings");
        dialog->setModal(true);
        dialog->resize(420, 300);
        dialog->setMinimumSize(360, 260);

        auto *layout = new QVBoxLayout(dialog);
        layout->setSpacing(0);
        layout->setContentsMargins(12, 12, 12, 12);

        auto *tabs = new QTabWidget(dialog);

        // -- Appearance tab --
        auto *appearanceTab = new QWidget();
        auto *appearanceLayout = new QVBoxLayout(appearanceTab);
        appearanceLayout->setSpacing(8);

        auto *themeLabel = new QLabel("Theme:");
        appearanceLayout->addWidget(themeLabel);

        auto *themeCombo = new QComboBox(appearanceTab);
        themeCombo->addItems({"Raycast", "Discord"});
        themeCombo->setCurrentText(ThemeManager::instance().currentTheme().name);
        appearanceLayout->addWidget(themeCombo);

        auto *saveThemeBtn = new QPushButton("Apply Theme", appearanceTab);
        appearanceLayout->addWidget(saveThemeBtn);
        appearanceLayout->addStretch();
        tabs->addTab(appearanceTab, "Appearance");

        // -- Connection tab --
        auto *connectionTab = new QWidget();
        auto *connectionLayout = new QVBoxLayout(connectionTab);
        connectionLayout->setSpacing(8);

        auto *tokenLabel = new QLabel("Bot Token:");
        connectionLayout->addWidget(tokenLabel);

        auto *tokenEdit = new QLineEdit(connectionTab);
        tokenEdit->setEchoMode(QLineEdit::Password);
        tokenEdit->setText(m_savedToken);
        tokenEdit->setPlaceholderText("Bot token");
        connectionLayout->addWidget(tokenEdit);

        auto *saveTokenBtn = new QPushButton("Save & Reconnect", connectionTab);
        connectionLayout->addWidget(saveTokenBtn);
        connectionLayout->addStretch();
        tabs->addTab(connectionTab, "Connection");

        layout->addWidget(tabs);

        // Close button
        auto *closeBtn = new QPushButton("Close", dialog);
        layout->addWidget(closeBtn, 0, Qt::AlignRight);

        // Theme change — only updates stylesheet, doesn't touch token or reconnect
        connect(saveThemeBtn, &QPushButton::clicked, this, [this, themeCombo]() {
            QString newTheme = themeCombo->currentText();
            if (newTheme != ThemeManager::instance().currentTheme().name) {
                ThemeManager::instance().setTheme(newTheme);
                QSettings settings("Umbra", "discord-qt");
                settings.setValue("theme_name", newTheme);
                qApp->setStyleSheet(ThemeManager::instance().regenerateQSS());
            }
        });

        // Token change — only saves token and reconnects, doesn't touch theme
        connect(saveTokenBtn, &QPushButton::clicked, this, [this, tokenEdit]() {
            QString newToken = tokenEdit->text().trimmed();
            if (!newToken.isEmpty() && newToken != m_savedToken) {
                m_savedToken = newToken;
                QSettings settings("Umbra", "discord-qt");
                settings.setValue("bot_token", newToken);

                m_client.disconnect();
                m_loginBarWidget->setVisible(true);
                m_tokenInput->setText(newToken);
                m_loginBtn->setEnabled(true);
                m_tokenInput->setEnabled(true);
                statusBar()->showMessage("Reconnecting...");
                m_client.login(newToken);
            }
        });

        connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);

        dialog->exec();
    }

    void onReady()
    {
        auto self = m_client.cache()->self();
        QString tag = self.globalName.value_or(self.username);
        setWindowTitle(QString("Umbra — %1").arg(tag));

        auto guilds = m_client.cache()->allGuilds();
        for (const auto &guild : guilds) {
            auto *item = new QListWidgetItem(guild.name, m_serverList);
            item->setData(Qt::UserRole, guild.id.toString());
            item->setToolTip(guild.name);
        }

        m_loginBarWidget->setVisible(false);
        m_messageInput->setEnabled(true);
        m_sendBtn->setEnabled(true);

        statusBar()->showMessage(QString("Connected as %1 · %2 servers").arg(tag, QString::number(guilds.size())));

        if (m_serverList->count() > 0) {
            m_serverList->setCurrentRow(0);
        }
    }

    void onServerSelected(int row)
    {
        if (row < 0) return;
        auto *item = m_serverList->item(row);
        if (!item) return;

        Snowflake guildId(item->data(Qt::UserRole).toString());
        m_channelLabel->setText(item->text());
        m_channelTree->clear();
        m_categoryItems.clear();

        auto channels = m_client.cache()->guildChannels(guildId);
        std::sort(channels.begin(), channels.end(), [](const Channel &a, const Channel &b) {
            if (a.type == ChannelType::GuildCategory && b.type != ChannelType::GuildCategory) return true;
            if (b.type == ChannelType::GuildCategory && a.type != ChannelType::GuildCategory) return false;
            return a.position < b.position;
        });

        for (const auto &ch : channels) {
            addChannelToTree(ch);
        }

        m_channelNameLabel->setVisible(false);
        m_messageView->clear();
        m_messageView->append(
            "<div class=\"empty-state\">"
            "<div class=\"empty-state-title\">Select a channel</div>"
            "<div class=\"empty-state-body\">Choose a text channel from the list to view messages.</div>"
            "</div>"
        );
        m_currentChannelId = Snowflake("");
        m_lastAuthorId.clear();
        m_messageCount = 0;
    }

    void onChannelTreeChanged(QTreeWidgetItem *current, QTreeWidgetItem *)
    {
        if (!current) return;
        QString type = current->data(0, Qt::UserRole + 1).toString();
        if (type != "channel") return;

        Snowflake channelId(current->data(0, Qt::UserRole).toString());
        m_currentChannelId = channelId;
        m_channelNameLabel->setText(current->text(0));
        m_channelNameLabel->setVisible(true);
        m_messageInput->setEnabled(true);
        m_sendBtn->setEnabled(true);

        m_oldestMessageId.reset();
        m_hasMoreMessages = false;
        m_isLoadingOlderMessages = false;
        m_loadMoreBtn->setVisible(false);

        m_messageView->clear();
        m_messageView->append(QString(
            "<div class=\"loading-skeleton\">"
            "<div class=\"skeleton-row\"></div>"
            "<div class=\"skeleton-row\"></div>"
            "<div class=\"skeleton-row short\"></div>"
            "</div>"
        ));

        m_client.fetchChannelMessages(channelId, 50);
    }

    void onLoadMoreClicked()
    {
        if (!m_oldestMessageId.has_value() || m_currentChannelId.toString().isEmpty()) return;

        m_isLoadingOlderMessages = true;
        m_loadMoreBtn->setEnabled(false);
        m_loadMoreBtn->setText("Loading...");

        m_client.fetchChannelMessages(m_currentChannelId, 50, m_oldestMessageId.value());
    }

    void onSendClicked()
    {
        QString text = m_messageInput->text().trimmed();
        if (text.isEmpty() || m_currentChannelId.toString().isEmpty()) return;

        m_messageInput->clear();
        m_client.sendMessage(m_currentChannelId, text);
    }

    void onConnectionChanged(bool connected)
    {
        if (!connected) {
            statusBar()->showMessage("Disconnected");
            m_loginBtn->setEnabled(true);
            m_loginBtn->setText("Connect");
            m_tokenInput->setEnabled(true);
            m_messageInput->setEnabled(false);
            m_sendBtn->setEnabled(false);
        }
    }

    void onMessageReceived(const Message &msg)
    {
        if (m_currentChannelId.toString().isEmpty() ||
            msg.channelId.toString() != m_currentChannelId.toString()) {
            return;
        }
        appendMessage(msg);
        m_messageView->moveCursor(QTextCursor::End);
    }

    void onMessageSent(const Message &msg)
    {
        if (m_currentChannelId.toString().isEmpty() ||
            msg.channelId.toString() != m_currentChannelId.toString()) {
            return;
        }
        appendMessage(msg);
        m_messageView->moveCursor(QTextCursor::End);
    }

    void onError(const QString &message)
    {
        m_messageView->append(QString("<span style=\"color: %1;\">Error: %2</span>")
            .arg(ThemeManager::instance().currentTheme().accentRed, message));
        statusBar()->showMessage(QString("Error: %1").arg(message));
        m_loginBtn->setEnabled(true);
        m_loginBtn->setText("Connect");
        m_tokenInput->setEnabled(true);
    }

private:
    void appendMessage(const Message &msg)
    {
        bool isContinuation = (m_lastAuthorId == msg.author.id.toString() && m_messageCount > 0);
        m_lastAuthorId = msg.author.id.toString();
        m_messageCount++;

        QString tag = DiscordMarkdown::escapeHtml(msg.author.globalName.value_or(msg.author.username));
        QString timeStr = msg.timestamp.isValid() ? msg.timestamp.toString("HH:mm") : "";
        QColor authorColor = hashColor(msg.author.id.toString());

        if (!isContinuation) {
            QString header = QString(
                "<div class=\"message-header\" style=\"margin: 12px 0 2px 0;\">"
                "<span style=\"color: %1; font-weight: 600;\">%2</span>"
                "<span style=\"color: %3; font-size: 12px; margin-left: 6px;\">%4</span>"
                "</div>"
            ).arg(authorColor.name(), tag, ThemeManager::instance().currentTheme().mute, timeStr);
            m_messageView->append(header);

            if (msg.editedTimestamp.has_value()) {
                m_messageView->append(
                    QString("<span style=\"color: %1; font-size: 11px;\">(edited)</span>")
                        .arg(ThemeManager::instance().currentTheme().mute)
                );
            }
        }

        if (!msg.content.isEmpty()) {
            QString contentHtml = DiscordMarkdown::toHtml(msg.content);
            QString indent = isContinuation ? QStringLiteral(" style=\"margin-left: 0;\"") : QString();
            m_messageView->append(QString("<div class=\"message-content\"%1>%2</div>").arg(indent, contentHtml));
        }

        for (const auto &embed : msg.embeds) {
            m_messageView->append(EmbedRenderer::toHtml(embed));
        }

        for (const auto &attachment : msg.attachments) {
            m_messageView->append(AttachmentRenderer::toHtml(attachment));
        }

        if (msg.content.isEmpty() && msg.embeds.isEmpty() && msg.attachments.isEmpty()) {
            m_messageView->append(
                QString("<div class=\"message-content\" style=\"color: %1;\">[empty message]</div>")
                    .arg(ThemeManager::instance().currentTheme().mute)
            );
        }
    }

    Client m_client;
    QListWidget *m_serverList;
    QTreeWidget *m_channelTree;
    QFrame *m_channelFrame;
    QLabel *m_channelLabel;
    QLabel *m_channelNameLabel;
    QWidget *m_loginBarWidget;
    QLineEdit *m_tokenInput;
    QLineEdit *m_messageInput;
    QPushButton *m_loginBtn;
    QPushButton *m_sendBtn;
    QToolButton *m_settingsBtn;
    QTextEdit *m_messageView;
    QPushButton *m_loadMoreBtn;
    Snowflake m_currentChannelId;
    QHash<QString, QTreeWidgetItem *> m_categoryItems;
    QString m_savedToken;
    QString m_lastAuthorId;
    int m_messageCount = 0;
    std::optional<Snowflake> m_oldestMessageId;
    bool m_hasMoreMessages = false;
    bool m_isLoadingOlderMessages = false;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Umbra");
    app.setApplicationVersion("0.1.0");
    app.setStyle("Fusion");

    // Load saved theme
    QSettings settings("Umbra", "discord-qt");
    QString savedTheme = settings.value("theme_name", "Raycast").toString();
    ThemeManager::instance().setTheme(savedTheme);

    app.setStyleSheet(ThemeManager::instance().regenerateQSS());

    MainWindow window;
    window.show();

    return app.exec();
}

#include "main.moc"
