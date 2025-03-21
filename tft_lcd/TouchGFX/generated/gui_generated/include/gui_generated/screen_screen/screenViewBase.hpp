/*********************************************************************************/
/********** THIS FILE IS GENERATED BY TOUCHGFX DESIGNER, DO NOT MODIFY ***********/
/*********************************************************************************/
#ifndef SCREENVIEWBASE_HPP
#define SCREENVIEWBASE_HPP

#include <gui/common/FrontendApplication.hpp>
#include <mvp/View.hpp>
#include <gui/screen_screen/screenPresenter.hpp>
#include <touchgfx/widgets/Box.hpp>
#include <touchgfx/widgets/TextArea.hpp>
#include <touchgfx/widgets/Image.hpp>
#include <touchgfx/widgets/ButtonWithLabel.hpp>
#include <touchgfx/widgets/TextAreaWithWildcard.hpp>

class screenViewBase : public touchgfx::View<screenPresenter>
{
public:
    screenViewBase();
    virtual ~screenViewBase();
    virtual void setupScreen();

protected:
    FrontendApplication& application() {
        return *static_cast<FrontendApplication*>(touchgfx::Application::getInstance());
    }

    /*
     * Member Declarations
     */
    touchgfx::Box __background;
    touchgfx::Box box1;
    touchgfx::TextArea textArea1;
    touchgfx::Image image1;
    touchgfx::Image image2;
    touchgfx::ButtonWithLabel redButton;
    touchgfx::ButtonWithLabel blueButton;
    touchgfx::ButtonWithLabel greenButton;
    touchgfx::TextArea textArea2;
    touchgfx::TextAreaWithOneWildcard textArea3;

    /*
     * Wildcard Buffers
     */
    static const uint16_t TEXTAREA3_SIZE = 20;
    touchgfx::Unicode::UnicodeChar textArea3Buffer[TEXTAREA3_SIZE];

private:

    /*
     * Callback Declarations
     */
    touchgfx::Callback<screenViewBase, const touchgfx::AbstractButton&> buttonCallback;

    /*
     * Callback Handler Declarations
     */
    void buttonCallbackHandler(const touchgfx::AbstractButton& src);

};

#endif // SCREENVIEWBASE_HPP
