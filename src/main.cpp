#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "ChatConfigPopup.hpp"

using namespace geode::prelude;

static std::vector<std::string> GD_PLAYERS = {
    // Streamers
    "Michigun", "Viprin", "Riot", "Juniper", "Wulzy",
    "EVW", "Doggie", "Nexus", "AeonAir", "Tride",
    "Cyclic", "Knobbelboy", "Sunix", "Technical49", "Dorami",
    "SpaceUK", "Diamond", "Trick", "Zoink", "Nswish",
    "Cursed", "BlassCFB", "MiKhaXx", "Mullsy", "Luqualizer",
    "Npesta", "xanii", "BTD6", "Cataclysm",
    "Krazyman50", "Zobros", "Sea1997", "Pennutoh", "FunnyGame",
    "TrusTa", "RicoLP", "ViPriN", "ChaSe", "Lemons",
    "Vortrox", "Triaxis",
    // Contributors
    "Axiom", "Human", "siniNight", "xblaze"
};

// static std::vector<bool> hasSpoken;
static const float CHAT_WIDTH = 135.0f;
static const float CHAT_HEIGHT = 170.0f;
static const float HEADER_HEIGHT = 16.0f;
static const float MSG_AREA_HEIGHT = CHAT_HEIGHT - HEADER_HEIGHT;
static const float MSG_LINE_HEIGHT = 11.0f;
static const float CHAT_PADDING = 4.0f;
static const float TEXT_SCALE = 0.27f;
static const int MAX_MESSAGES = 16;

static const ccColor3B TWITCH_COLORS[] = {
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {178, 70, 255},
    {255, 105, 180},
    {30, 144, 255},
    {0, 255, 127},
    {255, 165, 0},
    {255, 215, 0},
    {255, 127, 80},
    {100, 149, 237},
    {144, 238, 144},
    {255, 20, 147},
    {64, 224, 208},
    {255, 99, 71},
};
static const int TWITCH_COLOR_COUNT = 15;

static ccColor3B colorForUsername(const std::string& name) {
    size_t hash = 0;
    for (char c : name) hash = hash * 31 + c;
    return TWITCH_COLORS[hash % TWITCH_COLOR_COUNT];
}

static std::string randomPlayer() {
    return GD_PLAYERS[rand() % GD_PLAYERS.size()];
}

struct ChatMessage {
    std::string username;
    std::string text;
};

static ChatMessage parseChatMessage(const std::string& raw) {
    size_t colon = raw.find(": ");
    if (colon == std::string::npos) return {"", raw};
    return {raw.substr(0, colon), raw.substr(colon + 2)};
}

static std::string chat(const std::string& msg) {
    return randomPlayer() + ": " + msg;
}

std::string to_upper(std::string str) {
    std::ranges::transform(str, str.begin(), [](unsigned char c) {
        return std::toupper(c);
    });
    return str;
}

static std::string wrapText(const std::string& msg, int maxChars = 25, int firstLineOffset = 0) {
    std::string result;
    int lineLen = firstLineOffset;
    for (size_t i = 0; i < msg.size(); ) {
        size_t wordEnd = msg.find(' ', i);
        if (wordEnd == std::string::npos) wordEnd = msg.size();
        std::string word = msg.substr(i, wordEnd - i);

        while ((int)word.size() > maxChars) {
            int space = maxChars - lineLen;
            if (space <= 0) {
                if (!result.empty()) result += '\n';
                lineLen = 0;
                continue;
            }
            if (!result.empty() && lineLen > 0) result += '\n';
            result += word.substr(0, space);
            word = word.substr(space);
            result += '\n';
            lineLen = 0;
        }

        if (lineLen > 0 && lineLen + 1 + (int)word.size() > maxChars) {
            result += '\n';
            lineLen = 0;
        }
        if (lineLen > 0) { result += ' '; lineLen++; }
        result += word;
        lineLen += (int)word.size();

        i = wordEnd;
        while (i < msg.size() && msg[i] == ' ') i++;
    }
    return result;
}

static const std::vector<std::string> IDLE_MESSAGES = {
    "gl hf",
    "lets go",
    "you got this",
    "gg",
    "pog",
    "POGGERS",
    "lets gooo",
    "hype",
    "W",
    "W run",
    "bro is cooking",
    "no way",
    "actually insane",
    "craaaazy",
    "bro",
    "what",
    "lol",
    "sheesh",
    "clean",
    "smooth af",
    "frfr",
    "real",
    "godlike",
    "heat check",
    "HEAT",
    "actually going",
    "nah bro",
    "omg",
    "OMGG",
    "go go go",
    "this is it",
    "the run",
    "W player",
    "not dropping",
    "stay focused",
    "in the zone",
    "locked in",
    "monkaS",
    "prayge",
    "copium",
    // "he's actually doing it",
    "no shot",
    "bro woke up",
    "diff",
    "cooked",
    "certified W",
    "lets actually go",
    "i believe",
    "trust",
    "real ones watching",
    "OMFG",
    "actually poggers",
    "not missing",
    "clean inputs",
    "GG EZ",
    // "he's built different",
    "W grinder",
    "insane player",
    "demon time",
    // "he's not stopping",
};

// FLUKE MESSAGES (NEW FEATURE!)
static const std::vector<std::string> FLUKE_MESSAGES = {
    "FLUKE",
    "ACTUAL FLUKE",
    "BRO WHAT",
    "FLUKE RUN INCOMING",
    "NO SHOT",
    "WHAT IS HAPPENING",
    "BRO TURNED IT ON",
    "FLUKE ACTIVATED",
    "CERTIFIED FLUKE",
    "LMAOOOO",
    "HES COOKING NOW",
    "FLUKE MERCHANT",
    "WHAT A TURNAROUND",
    "THIS IS INSANE",
    "PLOT TWIST",
    "FLUKE ALERT",
};

static const std::vector<std::string> START_MESSAGES = {
    // "he's starting",
    "here we go",
    "attempt time",
    "lets see it",
    "W incoming",
    "is this the run",
    "woke up and chose violence",
    "good luck bro",
    "gl gl gl",
    "GLHF",
    "you got this fr",
    "the grind continues",
    "back at it",
    "W attempt pls",
    "another one",
    // "let "+fields->m_him+" cook",
    "cooking rn",
    "o7",
    "real",
    "pog",
};

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        CCNodeRGBA* m_chatRoot = nullptr;
        CCLayerColor* m_chatBg = nullptr;
        CCLayerColor* m_header = nullptr;
        CCLabelBMFont* m_headerLabel = nullptr;
        CCLabelBMFont* m_liveLabel = nullptr;
        CCLayerColor* m_liveBadge = nullptr;
        CCNode* m_msgContainer = nullptr;
        std::vector<CCNode*> m_messageRows;
        std::vector<int> m_rowHeights;
        float m_randomChatTimer = 0.0f;
        float m_nextChatDelay = 1.8f;
        float m_deathChatTimer = 0.0f;
        float m_deathSpamDuration = 2.0f;
        bool m_isDeathSpamming = false;
        float holdPercent = 22.0f;
        float goPercent = 37.0f;
        float superGoPercent = 80.0f;
        float ggPercent = 99.9999f;
        int att = 0;
        bool enabled = false;
        bool m_positiveChat = false;
        bool m_echoClipPresent = false;
        bool m_clipMessageFired = false;
        float m_bestPercent = 0.0f;
        float m_idleChatTimer = 0.0f;
        float m_nextIdleDelay = 2.8f;
        int m_numViewers = 69;
        float m_lastAttPercent = 0.0f;
        float m_opacity = 1.0f;
        GJGameLevel* m_lvl;
        std::string m_font = "bigFont";
        std::string m_he = "he";
        std::string m_him = "him";
        // FLUKE DETECTION (NEW):
        float m_lowestPercent = 100.0f;
        bool m_hadEarlyDeath = false;
        float m_deathPercent = 0.0f;
        bool m_flukeDetected = false;
    };

public:
    void reloadThresholds() {
        if (!m_level) return;
        auto fields = m_fields.self();
        fields->holdPercent = loadPercentForLevel(m_level->m_levelID, "hold-percent", 22.0f);
        fields->goPercent = loadPercentForLevel(m_level->m_levelID, "go-percent", 37.0f);
        fields->superGoPercent = loadPercentForLevel(m_level->m_levelID, "supergo-percent", 80.0f);
        fields->ggPercent = loadPercentForLevel(m_level->m_levelID, "gg-percent", 99.9999f);
        fields->enabled = loadDisabledForLevel(m_level->m_levelID, "enabled", true);
    }

    void addChatMessage(const std::string& message) {
        auto fields = m_fields.self();
        if (!fields->m_chatRoot) return;

        auto chatMsg = parseChatMessage(message);
        auto label = CCLabelBMFont::create(
            wrapText(chatMsg.text, 25, chatMsg.username.length() + 2).c_str(),
            fields->m_font.c_str()
        );
        label->setScale(TEXT_SCALE);
        label->setColor(colorForUsername(chatMsg.username));

        auto row = CCNode::create();
        auto nameLabel = CCLabelBMFont::create(chatMsg.username.c_str(), fields->m_font.c_str());
        nameLabel->setScale(TEXT_SCALE);
        nameLabel->setColor(colorForUsername(chatMsg.username));
        nameLabel->setPosition({CHAT_PADDING, 0});
        row->addChild(nameLabel);

        auto textLabel = CCLabelBMFont::create(
            wrapText(chatMsg.text, 25, chatMsg.username.length() + 2).c_str(),
            fields->m_font.c_str()
        );
        textLabel->setScale(TEXT_SCALE);
        textLabel->setColor(colorForUsername(chatMsg.username));
        textLabel->setPosition({CHAT_PADDING + chatMsg.username.length() * 4.0f, 0});
        row->addChild(textLabel);

        float rowHeight = MSG_LINE_HEIGHT;
        row->setContentHeight(rowHeight);
        fields->m_msgContainer->addChild(row);
        fields->m_messageRows.push_back(row);
        fields->m_rowHeights.push_back(rowHeight);

        while (fields->m_messageRows.size() > MAX_MESSAGES) {
            auto oldRow = fields->m_messageRows.front();
            fields->m_messageRows.erase(fields->m_messageRows.begin());
            fields->m_rowHeights.erase(fields->m_rowHeights.begin());
            oldRow->removeFromParent();
        }

        float totalHeight = 0;
        for (int h : fields->m_rowHeights) totalHeight += h;

        float yPos = MSG_AREA_HEIGHT - MSG_LINE_HEIGHT;
        for (size_t i = 0; i < fields->m_messageRows.size(); i++) {
            fields->m_messageRows[i]->setPositionY(yPos);
            yPos -= fields->m_rowHeights[i];
        }
    }

    void reloadSettings() {
        auto fields = m_fields.self();
        fields->m_font = Mod::get()->getSettingValue<std::string>("font");
        fields->m_opacity = Mod::get()->getSettingValue<float>("opacity");
        fields->m_he = Mod::get()->getSettingValue<std::string>("he");
        fields->m_him = Mod::get()->getSettingValue<std::string>("him");
        if (fields->m_chatBg) fields->m_chatBg->setOpacity(fields->m_opacity * 255);
    }

    void setupChat() {
        auto fields = m_fields.self();

        fields->enabled = Mod::get()->getSettingValue<bool>("enabled-by-default");
        reloadSettings();
        fields->m_numViewers = Mod::get()->getSettingValue<int>("viewer-count");

        if (!fields->enabled) return;

        auto playLayer = PlayLayer::get();
        if (!playLayer) return;

        auto director = CCDirector::sharedDirector();
        auto winSize = director->getWinSize();

        fields->m_chatRoot = CCNodeRGBA::create();
        fields->m_chatRoot->setPosition({
            winSize.width - 150.0f + Mod::get()->getSettingValue<int>("x-off"),
            50.0f + Mod::get()->getSettingValue<int>("y-off")
        });

        fields->m_chatBg = CCLayerColor::create({50, 50, 50, 100}, CHAT_WIDTH, CHAT_HEIGHT);
        fields->m_chatRoot->addChild(fields->m_chatBg);

        fields->m_header = CCLayerColor::create({30, 30, 30, 150}, CHAT_WIDTH, HEADER_HEIGHT);
        fields->m_header->setPosition({0, CHAT_HEIGHT - HEADER_HEIGHT});
        fields->m_chatRoot->addChild(fields->m_header);

        fields->m_liveBadge = CCLayerColor::create({255, 0, 0, 200}, 20.0f, 10.0f);
        fields->m_liveBadge->setPosition({CHAT_WIDTH - 25, CHAT_HEIGHT - HEADER_HEIGHT + 3});
        fields->m_chatRoot->addChild(fields->m_liveBadge);

        fields->m_liveLabel = CCLabelBMFont::create("LIVE", "bigFont.fnt");
        fields->m_liveLabel->setScale(0.2f);
        fields->m_liveLabel->setPosition({CHAT_WIDTH - 15, CHAT_HEIGHT - HEADER_HEIGHT + 5});
        fields->m_chatRoot->addChild(fields->m_liveLabel);

        fields->m_headerLabel = CCLabelBMFont::create(
            (std::to_string(fields->m_numViewers) + " viewers").c_str(),
            "bigFont.fnt"
        );
        fields->m_headerLabel->setScale(0.25f);
        fields->m_headerLabel->setPosition({CHAT_PADDING, CHAT_HEIGHT - HEADER_HEIGHT / 2});
        fields->m_chatRoot->addChild(fields->m_headerLabel);

        fields->m_msgContainer = CCNode::create();
        fields->m_msgContainer->setContentSize({CHAT_WIDTH, MSG_AREA_HEIGHT});
        fields->m_msgContainer->setPosition({0, 0});
        fields->m_chatRoot->addChild(fields->m_msgContainer);

        this->m_uiLayer->addChild(fields->m_chatRoot);
    }

    void update(float dt) {
        PlayLayer::update(dt);
        auto fields = m_fields.self();

        

        float progress = this->getCurrentPercent();

        // FLUKE DETECTION (NEW)
        // Track the lowest percent reached
        if (progress < fields->m_lowestPercent) {
            fields->m_lowestPercent = progress;
        }
        
        // If died early, mark it
        if (progress < 20.0f) {
            fields->m_hadEarlyDeath = true;
            fields->m_deathPercent = progress;
        }
        
        // FLUKE TRIGGER!
        if (fields->m_hadEarlyDeath && 
            !fields->m_flukeDetected && 
            progress >= 50.0f &&
            fields->m_lowestPercent < 15.0f) {
            
            fields->m_flukeDetected = true;
            
            // SPAM FLUKE MESSAGES
            addChatMessage(chat(FLUKE_MESSAGES[rand() % FLUKE_MESSAGES.size()]));
            addChatMessage(chat(FLUKE_MESSAGES[rand() % FLUKE_MESSAGES.size()]));
            addChatMessage(chat(FLUKE_MESSAGES[rand() % FLUKE_MESSAGES.size()]));
        }

        if (!fields->enabled) return;

        if (this->m_isPracticeMode && !Mod::get()->getSettingValue<bool>("enabled-in-practice")) {
            return;
        }

        fields->m_lastAttPercent = progress;

        if (progress > 0) {
            fields->m_randomChatTimer += dt;
            std::vector<std::string> startMessages = START_MESSAGES;

            if (fields->att == 0 && fields->m_randomChatTimer < 1.8f) {
                if (fields->m_randomChatTimer > 0.5f && rand() % 3 == 0) {
                    addChatMessage(chat(startMessages[rand() % startMessages.size()]));
                }
            }

            if (progress < 5.0f) {
                fields->m_idleChatTimer = 0;
            } else if (progress < fields->holdPercent) {
                fields->m_idleChatTimer += dt;
                if (fields->m_idleChatTimer >= fields->m_nextIdleDelay) {
                    if (rand() % 2 == 0 && Mod::get()->getSettingValue<bool>("positive-chat")) {
                        std::vector<std::string> positiveMessages = {
                            chat("you're doing great"),
                            chat("keep it up"),
                            chat("you got this"),
                            chat("nice job so far"),
                            chat("looking good"),
                        };
                        addChatMessage(positiveMessages[rand() % positiveMessages.size()]);
                    } else {
                        addChatMessage(chat(IDLE_MESSAGES[rand() % IDLE_MESSAGES.size()]));
                    }
                    fields->m_idleChatTimer = 0;
                    fields->m_nextIdleDelay = 1.0f + (rand() % 30) / 10.0f / 100.0f * abs(fields->m_numViewers);
                    if (fields->m_nextIdleDelay > 2.8f) fields->m_nextIdleDelay = 2.8f;
                }
            }

            if (fields->m_echoClipPresent && !fields->m_clipMessageFired) {
                bool newBestPastGo = progress >= fields->goPercent && progress > fields->m_bestPercent - 0.01f && fields->att > 5;
                bool superGoNewBest = progress >= fields->superGoPercent;
                if (newBestPastGo || superGoNewBest) {
                    std::vector<std::string> clipMessages = {
                        chat("BRO CLIP THAT"),
                        chat("CLIP IT CLIP IT"),
                        chat("F6 F6 F6 F6"),
                        chat("CLIP THIS RIGHT NOW"),
                        chat("BRO HIT F6"),
                        chat("RECORDING???"),
                        chat("DONT FORGET TO CLIP"),
                        chat("AXIOM.ECHOCLIP MENTIONED"),
                    };
                    addChatMessage(clipMessages[rand() % clipMessages.size()]);
                    fields->m_clipMessageFired = true;
                }
            }

            // hold
            if (progress >= fields->holdPercent && progress < fields->goPercent) {
                fields->m_randomChatTimer += dt;
                if (fields->m_randomChatTimer >= fields->m_nextChatDelay) {
                    std::vector<std::string> messages = {
                        chat("holdlldldldl"),
                        chat("HOLD IT"),
                        chat("HOOOOOOOOLDDDDDDDDDDDD"),
                        chat("HOOOLD"),
                        chat("HOLDDDDDDDDDDDDDDDDDDD"),
                        chat("hold hold hold"),
                        chat("dont let go"),
                        chat("GRIP"),
                        chat("stay on it"),
                        chat("holding..."),
                    };
                    addChatMessage(messages[rand() % messages.size()]);
                    fields->m_randomChatTimer = 0;
                    float t = (progress - fields->holdPercent) / (fields->goPercent - fields->holdPercent);
                    fields->m_nextChatDelay = 0.2f - (t * 0.067f) / 100.0f * abs(fields->m_numViewers);
                }
            }
            // gooo
            else if (progress >= fields->goPercent && progress < fields->superGoPercent) {
                fields->m_randomChatTimer += dt;
                if (fields->m_randomChatTimer >= fields->m_nextChatDelay) {
                    std::vector<std::string> messages = {
                        chat("GOOOO"),
                        chat("LETS GOOOOO"),
                        chat("CMON"),
                        chat("GOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO"),
                        chat("GO GO GO GO"),
                        chat("YESYESYES"),
                        chat(to_upper(fields->m_he)+"S GOING"),
                        chat("ACTUALLY GOING"),
                        chat("DO NOT MISS"),
                        chat("CMON CMON CMON"),
                        chat("W W W W W"),
                        chat("LETSSSSS GOOOOO"),
                        chat("clean"),
                        chat(fields->m_he+" is NOT dropping"),
                    };
                    addChatMessage(messages[rand() % messages.size()]);
                    fields->m_randomChatTimer = 0;
                    float t = (progress - fields->goPercent) / (fields->superGoPercent - fields->goPercent);
                    fields->m_nextChatDelay = 0.133f - (t * 0.033f) / 100.0f * abs(fields->m_numViewers);
                }
            }
            // super go
            else if (progress >= fields->superGoPercent && progress < fields->ggPercent) {
                fields->m_randomChatTimer += dt;
                if (fields->m_randomChatTimer >= fields->m_nextChatDelay) {
                    std::vector<std::string> messages = {
                        chat("SUPER GOOOOOOOOOOOOOOOOOOOOOO"),
                        chat("SUPERGOOOOOOOOOOOOOOOOOOOOOOOO!!!!!!!!!!!!"),
                        chat("I WAS HEREEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"),
                        chat("CMONNNN"),
                        chat("GOOOOO GOOOOOO GOOOOOOOO"),
                        chat("BRO IS ACTUALLY DOING IT"),
                        chat("NO WAY NO WAY NO WAY"),
                        chat("CLEAN CLEAN CLEAN"),
                        chat("DONT MISS NOW"),
                        chat("SO CLOSE TO END"),
                        chat("END GAME BRO"),
                        chat("LITERALLY ENDING"),
                        chat("ALMOST THERE"),
                        chat("PLEASE"),
                        chat("OH MY GOD"),
                        chat("INSANE"),
                    };
                    addChatMessage(messages[rand() % messages.size()]);
                    fields->m_randomChatTimer = 0;
                    float t = (progress - fields->superGoPercent) / (99.9999f - fields->superGoPercent);
                    fields->m_nextChatDelay = 0.1f - (t * 0.033f) / 100.0f * abs(fields->m_numViewers);
                }
            }
            // 100%: gg
            else if (progress > fields->ggPercent) {
                fields->m_randomChatTimer += dt;
                if (fields->m_randomChatTimer >= fields->m_nextChatDelay) {
                    std::vector<std::string> messages = {
                        chat("GG"),
                        chat("GGS"),
                        chat("WWWWWWWWWWWWWWWWWWWWWWWWW"),
                        chat("LETS GOOOOO"),
                        chat("WOOOOOOOOOOOOO"),
                        chat(to_upper(fields->m_he)+" DID IT"),
                        chat("NO WAY BRO"),
                        chat("ACTUAL W"),
                        chat("POGGERS"),
                        chat("GG EZ"),
                        chat("CARRIED"),
                        chat("W PLAYER"),
                        chat("LEGENDARY"),
                        chat("GOAT"),
                        chat("BRO IS BUILT DIFFERENT"),
                        chat("CLIP IT"),
                        chat("HALL OF FAME"),
                    };
                    addChatMessage(messages[rand() % messages.size()]);
                    fields->m_randomChatTimer = 0;
                    fields->m_nextChatDelay = 0.033f + (rand() % 18) / 1000.0f / 100.0f * abs(fields->m_numViewers);
                }
            }
        }
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        PlayLayer::destroyPlayer(player, object);
        auto fields = m_fields.self();

        float progress = this->getCurrentPercent();
        std::string username = GameManager::sharedState()->m_playerName;

        if (username == "Axiom" && progress >= fields->goPercent && !fields->m_isDeathSpamming) {
            std::vector<std::string> axiomRoasts = {
                "Axiom: skill issue imo",
                "Axiom: bro",
                "Axiom: nah that's crazy",
                randomPlayer() + ": skill issue axiom",
                randomPlayer() + ": axiom diff",
                randomPlayer() + ": bro really died there",
                randomPlayer() + ": AXIOM NOOOO",
                randomPlayer() + ": how did u die there axiom",
                randomPlayer() + ": certified axiom moment",
                randomPlayer() + ": this is why echochoke exists",
                randomPlayer() + ": axiom.echochoke is going to go crazy",
                randomPlayer() + ": dev couldnt beat his own level lmao",
            };
            addChatMessage(axiomRoasts[rand() % axiomRoasts.size()]);
        }

        if (fields->att > 16 && !fields->m_isDeathSpamming) {
            fields->m_isDeathSpamming = true;
            float t = progress / 100.0f;
            fields->m_deathSpamDuration = 2.0f + (t * t * 12.0f);
            fields->m_nextChatDelay = 0.5f - (t * 0.49f) / 100 * abs(fields->m_numViewers);
        }
        fields->att += 1;
        fields->m_clipMessageFired = false;
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        auto fields = m_fields.self();
        
        // RESET FLUKE TRACKING (NEW):
        fields->m_lowestPercent = 100.0f;
        fields->m_flukeDetected = false;
        // Note: We DON'T reset m_hadEarlyDeath or m_deathPercent
        // because we want to remember the previous attempt's death!
    }
};

class $modify(MyPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto rightMenu = this->getChildByID("right-button-menu");
        if (!rightMenu) {
            log::error("right-button-menu not found");
            return;
        }

        auto myButtonSprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn02_001.png");
        auto myButton = CCMenuItemSpriteExtra::create(
            myButtonSprite,
            this,
            menu_selector(MyPauseLayer::onMyButton)
        );

        rightMenu->addChild(myButton);
        static_cast<CCMenu*>(rightMenu)->updateLayout();
    }

    void onMyButton(CCObject*) {
        ChatConfigPopup::create()->show();
    }
};

class $modify(MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        std::thread([]() {
            auto res = web::WebRequest().getSync("https://badges.hiimjasmine00.com/developer");
            if (!res.ok()) {
                log::error("Failed to fetch dev list: {}", res.code());
                return;
            }
            auto arr = res.json().unwrapOr(matjson::Value{})
                .asArray().unwrapOr(std::vector<matjson::Value>{});
            for (auto& entry : arr) {
                GD_PLAYERS.push_back(entry["name"].asString().unwrapOrDefault());
            }
            std::sort(GD_PLAYERS.begin(), GD_PLAYERS.end());

            auto raw = Mod::get()->getSettingValue<std::string>("custom-chatters");
            std::stringstream ss(raw);
            std::string name;
            while (std::getline(ss, name, ' ')) {
                if (!name.empty()) GD_PLAYERS.push_back(name);
            }

            GD_PLAYERS.erase(std::unique(GD_PLAYERS.begin(), GD_PLAYERS.end()), GD_PLAYERS.end());
        }).detach();
        return true;
    }
};

void reloadPlayLayerThresholds() {
    if (auto pl = PlayLayer::get())
        static_cast<MyPlayLayer*>(pl)->reloadThresholds();
}
