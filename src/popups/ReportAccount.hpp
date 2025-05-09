#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/utils/web.hpp>
#include <fmt/format.h>

using namespace geode::prelude;

class ReportAccount : public Popup<> {
    int accID{ 0 };
    TextInput* input{ nullptr };
    EventListener<web::WebTask> reportListener;

    bool setup() override {
        setTitle("Report Account to AbuseDB");
        scheduleOnce(schedule_selector(ReportAccount::addContent), 0.1f);
        return true;
    }

    void addContent(float) {
        auto size = getContentSize();
        input = TextInput::create(260.f, "Enter reason for report", "chatFont.fnt");
        input->setPosition({ size.width / 2, size.height / 2 + 20.f });
        addChild(input);

        auto menu = CCMenu::createWithItem(CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Send Report"),
            this, menu_selector(ReportAccount::onSend)
        ));
        menu->setPosition({ size.width / 2, size.height / 2 - 40.f });
        addChild(menu);
    }

    void onSend(CCObject*) {
        if (!GJAccountManager::get()) {
            return;
        }

        auto reason = input->getString();

        web::WebRequest req;
        req.header("Content-Type", "application/x-www-form-urlencoded")
           .bodyString(fmt::format("creator={}&author={}&reason={}",
                                   accID, GJAccountManager::get()->m_accountID, reason));
        reportListener.bind([](web::WebTask::Event* e) {
            if (e->getProgress())
                return;
            if (e->isCancelled())
                return;
            if (auto res = e->getValue(); res && res->code() >= 200 && res->code() < 300) {
                FLAlertLayer::create("Success", "Report sent successfully.", "OK")->show();
            } else {
                FLAlertLayer::create("Error", "Failed to send report. Please try again.", "OK")->show();
            }
        });
        reportListener.setFilter(req.post("https://abusedb.dpdns.org/api/report"));
    }

public:
    static ReportAccount* create(int accountID) {
        auto ret = new ReportAccount();
        ret->accID = accountID;
        if (ret && ret->initAnchored(400.f, 200.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};