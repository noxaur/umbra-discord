#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QGroupBox>
#include <QStatusBar>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QListWidget>
#include <QSplitter>
#include <QFrame>
#include <QScrollArea>
#include "core/client.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr)
        : QMainWindow(parent)
    {
        setWindowTitle("Discord Qt");
        resize(900, 600);

        // Central splitter: server list | channel list | messages
        auto *splitter = new QSplitter(Qt::Horizontal, this);
        setCentralWidget(splitter);

        // --- Server list (left sidebar) ---
        auto *serverFrame = new QFrame(splitter);
        auto *serverLayout = new QVBoxLayout(serverFrame);
        serverLayout->setContentsMargins(0, 0, 0, 0);

        m_serverList = new QListWidget(serverFrame);
        m_serverList->setMaximumWidth(180);
        m_serverList->setMinimumWidth(120);
        serverLayout->addWidget(m_serverList);

        splitter->addWidget(serverFrame);

        // --- Channel list (middle sidebar) ---
        auto *channelFrame = new QFrame(splitter);
        auto *channelLayout = new QVBoxLayout(channelFrame);
        channelLayout->setContentsMargins(0, 0, 0, 0);

        m_channelLabel = new QLabel("Select a server", channelFrame);
        m_channelLabel->setStyleSheet("font-weight: bold; padding: 8px;");
        channelLayout->addWidget(m_channelLabel);

        m_channelList = new QListWidget(channelFrame);
        m_channelList->setMaximumWidth(200);
        m_channelList->setMinimumWidth(140);
        channelLayout->addWidget(m_channelList);

        splitter->addWidget(channelFrame);

        // --- Main content (right) ---
        auto *mainWidget = new QWidget(splitter);
        auto *mainLayout = new QVBoxLayout(mainWidget);
        mainLayout->setContentsMargins(4, 4, 4, 4);

        // Login bar (shown until connected)
        auto *loginBar = new QHBoxLayout();
        m_tokenInput = new QLineEdit(mainWidget);
        m_tokenInput->setEchoMode(QLineEdit::Password);
        m_tokenInput->setPlaceholderText("Token only — no 'Bot ' prefix");
        loginBar->addWidget(m_tokenInput);

        m_loginBtn = new QPushButton("Connect", mainWidget);
        loginBar->addWidget(m_loginBtn);
        mainLayout->addLayout(loginBar);

        // Status bar (top of main area)
        m_statusBar = new QStatusBar(this);
        setStatusBar(m_statusBar);
        m_statusBar->showMessage("Not connected");

        // Message view
        m_messageView = new QTextEdit(mainWidget);
        m_messageView->setReadOnly(true);
        m_messageView->setStyleSheet("background-color: #36393f; color: #dcddde; font-family: monospace; font-size: 13px;");
        mainLayout->addWidget(m_messageView);

        // Message input
        auto *inputLayout = new QHBoxLayout();
        m_channelNameLabel = new QLabel("No channel selected", mainWidget);
        m_channelNameLabel->setStyleSheet("font-weight: bold;");
        inputLayout->addWidget(m_channelNameLabel);

        m_messageInput = new QLineEdit(mainWidget);
        m_messageInput->setPlaceholderText("Type a message...");
        m_messageInput->setEnabled(false);
        inputLayout->addWidget(m_messageInput);

        m_sendBtn = new QPushButton("Send", mainWidget);
        m_sendBtn->setEnabled(false);
        inputLayout->addWidget(m_sendBtn);
        mainLayout->addLayout(inputLayout);

        splitter->addWidget(mainWidget);

        // Set splitter proportions
        splitter->setSizes({160, 180, 560});

        // Menu bar
        auto *menuBar = this->menuBar();
        auto *helpMenu = menuBar->addMenu("&Help");
        auto *aboutAction = helpMenu->addAction("&About");
        connect(aboutAction, &QAction::triggered, this, [this]() {
            QMessageBox::about(this, "About DiscordQt",
                "Discord Qt Core v0.1.0\n\n"
                "A Discord client built with Qt6 and C++.\n"
                "Enter your bot token and click Connect.");
        });

        // Client setup
        m_client.setIntents(Intent::Guilds | Intent::GuildMessages | Intent::DirectMessages);

        // Signal connections
        connect(m_loginBtn, &QPushButton::clicked, this, &MainWindow::onLoginClicked);
        connect(m_sendBtn, &QPushButton::clicked, this, &MainWindow::onSendClicked);
        connect(m_messageInput, &QLineEdit::returnPressed, this, &MainWindow::onSendClicked);

        connect(m_serverList, &QListWidget::currentRowChanged, this, &MainWindow::onServerSelected);
        connect(m_channelList, &QListWidget::currentRowChanged, this, &MainWindow::onChannelSelected);

        connect(&m_client, &Client::ready, this, &MainWindow::onReady);
        connect(&m_client, &Client::connectionStateChanged, this, &MainWindow::onConnectionChanged);
        connect(&m_client, &Client::messageReceived, this, &MainWindow::onMessageReceived);
        connect(&m_client, &Client::error, this, &MainWindow::onError);
    }

private slots:
    void onLoginClicked()
    {
        QString token = m_tokenInput->text().trimmed();
        if (token.isEmpty()) {
            QMessageBox::warning(this, "Error", "Please enter a bot token.");
            return;
        }
        m_loginBtn->setEnabled(false);
        m_tokenInput->setEnabled(false);
        m_statusBar->showMessage("Connecting...");
        appendMessage("Connecting to Discord...");
        m_client.login(token);
    }

    void onReady()
    {
        m_statusBar->showMessage("Connected");
        appendMessage("Connected to Discord.");

        auto self = m_client.cache()->self();
        QString tag = self.discriminator.has_value()
            ? QString("%1#%2").arg(self.username, *self.discriminator)
            : self.username;
        setWindowTitle(QString("Discord Qt — %1").arg(tag));

        // Populate server list from cache
        auto guilds = m_client.cache()->allGuilds();
        for (const auto &guild : guilds) {
            auto *item = new QListWidgetItem(guild.name, m_serverList);
            item->setData(Qt::UserRole, guild.id.toString());
        }

        m_loginBtn->setText("Connected");
        m_messageInput->setEnabled(true);
        m_sendBtn->setEnabled(true);
    }

    void onServerSelected(int row)
    {
        if (row < 0) return;
        auto *item = m_serverList->item(row);
        if (!item) return;

        Snowflake guildId(item->data(Qt::UserRole).toString());
        m_channelLabel->setText(item->text());
        m_channelList->clear();

        auto channels = m_client.cache()->guildChannels(guildId);
        for (const auto &ch : channels) {
            if (ch.type == ChannelType::Text || ch.type == ChannelType::DM) {
                auto *item = new QListWidgetItem("# " + ch.name, m_channelList);
                item->setData(Qt::UserRole, ch.id.toString());
            }
        }

        if (m_channelList->count() > 0) {
            m_channelList->setCurrentRow(0);
        }
    }

    void onChannelSelected(int row)
    {
        if (row < 0) return;
        auto *item = m_channelList->item(row);
        if (!item) return;

        Snowflake channelId(item->data(Qt::UserRole).toString());
        m_currentChannelId = channelId;
        m_channelNameLabel->setText("# " + item->text().mid(2)); // strip "# "
        m_messageInput->setEnabled(true);
        m_sendBtn->setEnabled(true);

        m_messageView->clear();
        appendMessage("Loading messages...");

        // Fetch messages from REST
        m_client.fetchChannelMessages(channelId, 50);
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
            m_statusBar->showMessage("Disconnected");
            appendMessage("Disconnected from Discord.");
            m_loginBtn->setEnabled(true);
            m_loginBtn->setText("Connect");
            m_tokenInput->setEnabled(true);
            m_messageInput->setEnabled(false);
            m_sendBtn->setEnabled(false);
        }
    }

    void onMessageReceived(const Message &msg)
    {
        // Only show messages for the current channel
        if (m_currentChannelId.toString().isEmpty() ||
            msg.channelId.toString() != m_currentChannelId.toString()) {
            // Still log it
            appendMessage(QString("[#%1] %2: %3")
                .arg(msg.channelId.toString().mid(0, 8), msg.author.username, msg.content));
            return;
        }
        appendMessage(QString("%2: %3")
            .arg(msg.author.username, msg.content));
    }

    void onError(const QString &message)
    {
        appendMessage("Error: " + message);
        m_statusBar->showMessage("Error");
        m_loginBtn->setEnabled(true);
        m_loginBtn->setText("Connect");
        m_tokenInput->setEnabled(true);
    }

private:
    void appendMessage(const QString &text)
    {
        m_messageView->append(text);
    }

    Client m_client;
    QListWidget *m_serverList;
    QListWidget *m_channelList;
    QLabel *m_channelLabel;
    QLabel *m_channelNameLabel;
    QLineEdit *m_tokenInput;
    QLineEdit *m_messageInput;
    QPushButton *m_loginBtn;
    QPushButton *m_sendBtn;
    QTextEdit *m_messageView;
    QStatusBar *m_statusBar;
    Snowflake m_currentChannelId;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("DiscordQt");
    app.setApplicationVersion("0.1.0");

    // Dark-ish theme
    app.setStyle("Fusion");

    MainWindow window;
    window.show();

    return app.exec();
}

#include "main.moc"
