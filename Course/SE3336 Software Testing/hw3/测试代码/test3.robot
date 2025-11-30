*** Settings ***
Library    SeleniumLibrary
Library    D:/Horizon/Projects/Ongoing/STest/outlook/resources/image_comparison_library.py
Resource    ./variables.robot    # 假设变量存储在外部文件

*** Variables ***
${OUTLOOK_URL}    https://outlook.live.com/
${BASELINE_IMAGE}    D:/Horizon/Projects/Ongoing/STest/outlook/img/baseline.png
${CURRENT_IMAGE}    D:/Horizon/Projects/Ongoing/STest/outlook/img/current.png
${OUTPUT_IMAGE}  D:/Horizon/Projects/Ongoing/STest/outlook/img/output.png
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
${JUNK_EMAIL}    xpath=//*[@data-folder-name="垃圾邮件"]
${CHOOSE_EMAIL}    xpath=//*[@aria-label="选择"]
${DELETE_BUTTON}    xpath=//button[@aria-label="删除"]
${SETTING_BUTTON}    xpath=//*[@aria-label="设置"]

*** Test Cases ***
Outlook 个人设置界面视觉回归测试
    [Documentation]    测试 Outlook 个人设置界面是否一致
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

    # 6. 点击设置按钮
    Wait Until Element Is Visible   ${SETTING_BUTTON}    10s
    Click Element    ${SETTING_BUTTON}

    Sleep   7s

    # 截图并保存为当前图片
    Capture Page Screenshot    ${CURRENT_IMAGE}

    # 对比图片
    ${result}=    Compare Images    ${BASELINE_IMAGE}    ${CURRENT_IMAGE}
    Highlight Differences    ${BASELINE_IMAGE}    ${CURRENT_IMAGE}   ${OUTPUT_IMAGE}
    Run Keyword If    not ${result}    Highlight Differences    ${BASELINE_IMAGE}    ${CURRENT_IMAGE}   ${OUTPUT_IMAGE}
    ...    ELSE    Log    图片对比通过，界面一致

    Close Browser

*** Keywords ***
Compare Images
    [Arguments]    ${baseline_path}    ${current_path}
    ${result}=    Evaluate    image_comparison_library.ImageComparisonLibrary.compare_images('${baseline_path}', '${current_path}')    modules=image_comparison_library
    [Return]    ${result}

Highlight Differences
    [Arguments]    ${baseline_path}    ${current_path}     ${output_path}
    Evaluate    image_comparison_library.ImageComparisonLibrary.highlight_differences('${baseline_path}', '${current_path}','${OUTPUT_IMAGE}')    modules=image_comparison_library