#include "ChatConfigPopup.hpp"

std::string levelKey(int levelID, const char* suffix) {
    return std::to_string(levelID) + suffix;
}

float loadPercentForLevel(int levelID, const char* suffix, float defaultValue) {
    auto key = levelKey(levelID, suffix);
    auto legacyValue = Mod::get()->getSavedValue<int>(key, static_cast<int>(defaultValue));
    return Mod::get()->getSavedValue<float>(key, static_cast<float>(legacyValue));
}

bool loadDisabledForLevel(int levelID, const char* suffix, bool defaultValue) {
    auto key = levelKey(levelID, suffix);
    return Mod::get()->getSavedValue<bool>(key, defaultValue);
}

bool ChatConfigPopup::setup() {
    this->setTitle("ChatGD Config");
    auto center = m_mainLayer->getContentSize() / 2;

    float holdPercent = 22.0f;
    float goPercent = 37.0f;
    float superGoPercent = 80.0f;
    bool enabled = true;

    if (auto playLayer = PlayLayer::get(); playLayer && playLayer->m_level) {
        auto levelID = playLayer->m_level->m_levelID;
        holdPercent = loadPercentForLevel(levelID, "hold-percent", 22.0f);
        goPercent = loadPercentForLevel(levelID, "go-percent", 37.0f);
        superGoPercent = loadPercentForLevel(levelID, "supergo-percent", 80.0f);
        enabled = loadDisabledForLevel(levelID, "enabled", true);
    }

    auto label1 = CCLabelBMFont::create("Hold %:", "bigFont.fnt");
    label1->setPosition({center.width - 120, center.height + 40});
    label1->setScale(0.3f);
    m_mainLayer->addChild(label1);

    m_textInput1 = geode::TextInput::create(200.0f, "");
    m_textInput1->setPosition({center.width + 30, center.height + 40});
    m_textInput1->setFilter("0123456789");
    m_textInput1->setMaxCharCount(3);
    m_textInput1->setString(std::to_string(static_cast<int>(holdPercent)));
    m_mainLayer->addChild(m_textInput1);

    auto label2 = CCLabelBMFont::create("Go %:", "bigFont.fnt");
    label2->setPosition({center.width - 127, center.height});
    label2->setScale(0.3f);
    m_mainLayer->addChild(label2);

    m_textInput2 = geode::TextInput::create(200.0f, "");
    m_textInput2->setPosition({center.width + 30, center.height});
    m_textInput2->setFilter("0123456789");
    m_textInput2->setMaxCharCount(3);
    m_textInput2->setString(std::to_string(static_cast<int>(goPercent)));
    m_mainLayer->addChild(m_textInput2);

    auto label3 = CCLabelBMFont::create("Super Go %:", "bigFont.fnt");
    label3->setPosition({center.width - 111, center.height - 40});
    label3->setScale(0.3f);
    m_mainLayer->addChild(label3);

    m_textInput3 = geode::TextInput::create(200.0f, "");
    m_textInput3->setPosition({center.width + 30, center.height - 40});
    m_textInput3->setFilter("0123456789");
    m_textInput3->setMaxCharCount(3);
    m_textInput3->setString(std::to_string(static_cast<int>(superGoPercent)));
    m_mainLayer->addChild(m_textInput3);

    auto label4 = CCLabelBMFont::create("Enabled:", "bigFont.fnt");
    label4->setPosition({center.width - 118, center.height - 80});
    label4->setScale(0.3f);
    m_mainLayer->addChild(label4);

    auto toggleMenu = CCMenu::create();
    toggleMenu->setPosition({center.width + 30, center.height - 80});
    m_enableToggle = CCMenuItemToggler::createWithStandardSprites(
        this, menu_selector(ChatConfigPopup::onToggle), 0.6f
    );
    m_enableToggle->toggle(!enabled);
    toggleMenu->addChild(m_enableToggle);
    m_mainLayer->addChild(toggleMenu);

    return true;
}

void ChatConfigPopup::onToggle(CCObject*) {}

ChatConfigPopup* ChatConfigPopup::create() {
    auto ret = new ChatConfigPopup();
    if (ret->init(300.0f, 220.0f)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void ChatConfigPopup::onClose(CCObject* sender) {
    auto playLayer = PlayLayer::get();
    if (!playLayer || !playLayer->m_level) {
        log::error("ChatGD: Could not save config");
        geode::Popup::onClose(sender);
        return;
    }

    int levelID = playLayer->m_level->m_levelID;

    Mod::get()->setSavedValue(levelKey(levelID, "hold-percent"),
        geode::utils::numFromString<float>(m_textInput1->getString()).unwrapOrDefault());
    Mod::get()->setSavedValue(levelKey(levelID, "go-percent"),
        geode::utils::numFromString<float>(m_textInput2->getString()).unwrapOrDefault());
    Mod::get()->setSavedValue(levelKey(levelID, "supergo-percent"),
        geode::utils::numFromString<float>(m_textInput3->getString()).unwrapOrDefault());
    Mod::get()->setSavedValue(levelKey(levelID, "enabled"), !m_enableToggle->isToggled());

    geode::Popup::onClose(sender);
}