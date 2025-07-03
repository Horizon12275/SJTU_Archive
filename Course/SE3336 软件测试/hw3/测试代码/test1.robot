*** Settings ***
Library    SeleniumLibrary
Resource    ./variables.robot    # 假设变量存储在外部文件

*** Variables ***
${OUTLOOK_URL}    https://outlook.live.com/
${LOGIN_BUTTON}    xpath=//*[@id="c-shellmenu_custom_outline_newtab_signin_bhvr100_right"]
${EMAIL_INPUT}    //*[@id="usernameEntry"]
${NEXT_BUTTON}    //*[@id="view"]/div/div[3]/button
${PASSWORD_INPUT}    //*[@id="passwordEntry"]
${SIGNIN_BUTTON}    //*[@id="view"]/div/div[5]/button
${REMEMBER_ME_NO}    //*[@id="view"]/div/div[5]/button[2]
${NEW_EMAIL_BUTTON}    //*[@id="114-group"]/div/div[1]/div/div/span/button[2]/span/i/span
${MESSAGE_TYPE_DROPDOWN}    //*[@id="Ribbon-588Dropdown"]/div/ul/li/div/ul/li[1]/button
${EMAIL_OPTION}    //*[@id="Ribbon-588Dropdown"]/div/ul/li/div/ul/li[1]/button/div/span
${TO_INPUT}    xpath=//*[@id="0"]
${SUBJECT_INPUT}    xpath=//*[@id="docking_InitVisiblePart_0"]/div/div[3]/div[2]/span/input
${BODY_INPUT}    xpath=//*[@id="editorParent_1"]/div
${INSERT_LINK_BUTTON}    xpath=//*[@aria-label="链接"]//*[text()='插入超链接。 (Ctrl+K)']/../../..//*[@class="splitPrimaryButton root-602"]
${LINK_TEXT_INPUT}    xpath=//*[@id="displayTextInput"]
${LINK_URL_INPUT}    xpath=//*[@id="linkInput"]
${INSERT_BUTTON}    xpath=//*[@id="ok-1"]
${SEND_BUTTON}    xpath=//*[@aria-label="发送"]
${SENT_FOLDER}    xpath=//*[@data-folder-name="已发送邮件"]

*** Test Cases ***
发送邮件测试
    [Documentation]    测试Outlook发送邮件功能，包含所有必选和可选元素
    Open Browser    ${OUTLOOK_URL}    chrome
    Maximize Browser Window

    # 1. 点击登录按钮
    Wait Until Element Is Visible    ${LOGIN_BUTTON}    10s
    Click Element    ${LOGIN_BUTTON}

    Sleep   4s

    # 2. 切换至登录页面（处理新标签页）
    @{window_handles}=    Get Window Handles
    Switch Window    ${window_handles}[1]

    # 3. 输入邮箱、点击下一个按钮
    Wait Until Element Is Visible    ${EMAIL_INPUT}    10s
    Input Text    ${EMAIL_INPUT}    ${VALID_EMAIL}    # 变量需在variables.robot中定义
    Click Element    ${NEXT_BUTTON}

    # 4. 输入密码、点击下一个按钮
    Wait Until Element Is Visible    ${PASSWORD_INPUT}    10s
    Input Text    ${PASSWORD_INPUT}    ${VALID_PASSWORD}    # 变量需在variables.robot中定义
    Click Element    ${SIGNIN_BUTTON}

    # 5. 点击保持登录状态中的否按钮
    Wait Until Element Is Visible    ${REMEMBER_ME_NO}    5s
    Click Element    ${REMEMBER_ME_NO}

    # 6. 点击新邮件按钮右侧的下拉框、点击邮件
    Wait Until Element Is Visible    ${NEW_EMAIL_BUTTON}    10s
    Click Element    ${NEW_EMAIL_BUTTON}
    # 假设下拉框在点击后会显示选项
    Wait Until Element Is Visible    ${MESSAGE_TYPE_DROPDOWN}    5s
    Click Element    ${EMAIL_OPTION}

    # 等待5s
    Sleep    1s

    # 7. 输入收件人邮箱、添加主题、添加内容
    Wait Until Element Is Visible    ${TO_INPUT}    10s
    Click Element    ${TO_INPUT}
    Wait Until Element Is Visible   //*[text()='2309996590@qq.com']     10s
    Click Element    //*[text()='2309996590@qq.com']
    Input Text    ${SUBJECT_INPUT}    Test Subject
    Input Text    ${BODY_INPUT}    This is a test email body.

    # 9. 点击发送按钮
    Click Element    ${SEND_BUTTON}

    Sleep    5s

    # 10. 验证已发送邮件
    Wait Until Element Is Visible    ${SENT_FOLDER}    10s
    Click Element    ${SENT_FOLDER}

    # 等待邮件出现在已发送列表
    Wait Until Page Contains Element    xpath=//*[text()='Test Subject']    20s

    # 关闭浏览器
    Close Browser