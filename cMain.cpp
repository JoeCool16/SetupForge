#include "cMain.h"
#include <wx/stdpaths.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/process.h>
#include <wx/msw/registry.h>
#include <wx/msgdlg.h>
#include <regex>
#include <iostream>
#include <string>
#include <Windows.h>
#include <wx/dir.h>
#include <fstream>

wxBEGIN_EVENT_TABLE(cMain, wxFrame)
	EVT_BUTTON(1001, OnButtonClicked)
    EVT_BUTTON(1002, OnRunScriptClicked)
    EVT_BUTTON(1003, OnSaveListboxClicked)
    EVT_BUTTON(1004, OnOpenListboxClicked)
    EVT_BUTTON(1005, OnDeleteButtonClicked)
wxEND_EVENT_TABLE()

cMain::cMain() : wxFrame(nullptr, wxID_ANY, "Setup Forge", wxPoint(30, 30), wxSize(800, 600))
{
    wxIcon appIcon;
    appIcon.LoadFile("app_icon.ico", wxBITMAP_TYPE_ICO);
    SetIcon(appIcon);

    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

    // Dropdown (wxChoice) to select options
    choices.Add("Run an Exe");
    choices.Add("Insert File");
    choices.Add("Create Folder");
    choices.Add("Create File");
    choices.Add("Add/Edit Environment Variables");
    choices.Add("Add/Edit System Environment Variables");
    choices.Add("Checkpoint");
    m_choice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(150, 30), choices);
    vbox->Add(m_choice, 0, wxALIGN_LEFT | wxTOP, 10);  // Center horizontally, 10px from the top

    // Create a horizontal sizer for the buttons and progress bar
    wxBoxSizer* hboxButtons = new wxBoxSizer(wxHORIZONTAL);

    // "+" Button to add the selected option to the list
    m_btnAdd = new wxButton(this, 1001, "+", wxDefaultPosition, wxSize(50, 30));
    hboxButtons->Add(m_btnAdd, 0, wxRIGHT, 10);  // Add 10px of space to the right of "+"

    // "-" Button to delete the selected option
    m_btnDelete = new wxButton(this, 1005, "-", wxDefaultPosition, wxSize(50, 30));
    hboxButtons->Add(m_btnDelete, 0, wxRIGHT, 10);  // Add 10px of space to the right of "-"

    // Add a flexible spacer to push the progress bar to the center
    hboxButtons->AddStretchSpacer(1);  // Push everything after this to the right

    // Progress Bar
    m_progressBar = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxSize(380, 25));
    hboxButtons->Add(m_progressBar, 0, wxALIGN_CENTER_VERTICAL);  // Center the progress bar vertically

    // Add another flexible spacer after the progress bar (optional)
    hboxButtons->AddStretchSpacer(1);  // Ensure symmetry, but this is optional

    // Add the horizontal sizer to the main vertical sizer
    vbox->Add(hboxButtons, 0, wxEXPAND | wxTOP, 10);

    // Middle box (wxListBox) to display selected options
    m_listBox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(500, 300));
    vbox->Add(m_listBox, 1, wxALIGN_CENTER | wxALL, 10);  // Expandable and centered

    m_listBox->Bind(wxEVT_LISTBOX_DCLICK, &cMain::OnListBoxDoubleClick, this);
    m_listBox->Bind(wxEVT_LEFT_DOWN, &cMain::OnListBoxMouseDown, this);
    m_listBox->Bind(wxEVT_MOTION, &cMain::OnListBoxMouseMove, this);
    m_listBox->Bind(wxEVT_LEFT_UP, &cMain::OnListBoxMouseUp, this);

    // Box to hold the buttons for saving and opening listbox files
    wxBoxSizer* fileBox = new wxBoxSizer(wxHORIZONTAL);

    // Save Step File Button
    m_btnSaveListbox = new wxButton(this, 1003, "Save Step File", wxDefaultPosition, wxSize(100, 30));
    fileBox->Add(m_btnSaveListbox, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

    // Open Step File Button
    m_btnOpenListbox = new wxButton(this, 1004, "Open Step File", wxDefaultPosition, wxSize(100, 30));
    fileBox->Add(m_btnOpenListbox, 0, wxALIGN_CENTER_VERTICAL);

    vbox->Add(fileBox, 0, wxALIGN_CENTER | wxTOP, 10);  // Add fileBox above Run Steps button

    // Horizontal sizer for Run Steps Button and Textbox
    wxBoxSizer* saveBox = new wxBoxSizer(wxHORIZONTAL);

    // Run Steps Button
    m_btnSave = new wxButton(this, 1002, "Run Steps", wxDefaultPosition, wxSize(100, 30));
    saveBox->Add(m_btnSave, 0, wxALIGN_CENTER_VERTICAL);

    vbox->Add(saveBox, 0, wxALIGN_CENTER | wxTOP, 10);  // Below the fileBox

    // Set the sizer for the frame
    this->SetSizer(vbox);

}

cMain::~cMain()
{
}

void cMain::WriteToLog(const wxString& message)
{
    wxString logDirectory;

    // Check if a script was opened and directory is set
    if (!m_scriptDirectory.IsEmpty())
    {
        logDirectory = m_scriptDirectory + "\\";
    }
    else
    {
        // Default to executable directory if no script is opened
        logDirectory = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath() + "\\";
    }

    wxString logFilePath = logDirectory + "app_log.txt";

    // Open the log file in append mode
    std::ofstream logFile(logFilePath.mb_str(), std::ios::app);

    if (logFile.is_open())
    {
        // Get current timestamp
        wxDateTime now = wxDateTime::Now();
        wxString timestamp = now.FormatISOCombined(' ');  // Format: YYYY-MM-DD HH:MM:SS

        // Write log entry
        logFile << "[" << timestamp.mb_str() << "] " << message.mb_str() << std::endl;
        logFile.close();
    }
    else
    {
        wxMessageBox("Failed to open log file at: " + logFilePath, "Error", wxOK | wxICON_ERROR);
    }
}

void cMain::OnListBoxMouseDown(wxMouseEvent& event)
{
    // Capture the initial selection index when mouse is pressed
    m_draggedIndex = m_listBox->HitTest(event.GetPosition());
    event.Skip();
}

void cMain::OnListBoxMouseMove(wxMouseEvent& event)
{
    if (event.Dragging() && m_draggedIndex != wxNOT_FOUND)
    {
        // Determine where the item is being dragged to
        int targetIndex = m_listBox->HitTest(event.GetPosition());

        if (targetIndex != wxNOT_FOUND && targetIndex != m_draggedIndex)
        {
            wxString draggedItem = m_listBox->GetString(m_draggedIndex);

            // Remove the dragged item from the list and reinsert at the target position
            m_listBox->Delete(m_draggedIndex);
            m_listBox->Insert(draggedItem, targetIndex);

            // Update the new index position
            m_draggedIndex = targetIndex;

            // Select the moved item for better UX feedback
            m_listBox->SetSelection(m_draggedIndex);
        }
    }

    event.Skip();
}

void cMain::OnListBoxMouseUp(wxMouseEvent& event)
{
    // Reset the dragging state
    m_draggedIndex = wxNOT_FOUND;
    event.Skip();
}

// Run an Executable(.exe)
void cMain::RunExe(const wxString & exePath)
{
    // Execute the file directly
    int retCode = wxExecute(exePath, wxEXEC_SYNC);
    if (retCode == -1)
    {
        m_errorFlag = true;
        wxMessageBox("Failed to execute the program!", "Error", wxOK | wxICON_ERROR);
        WriteToLog("Failed to execute the program!");
    }
}

// Move a file
void cMain::FileMover(const wxString& sourcePath, const wxString& destinationPath)
{
    if (wxCopyFile(sourcePath, destinationPath))
    {
        m_listBox->AppendString("Moved: " + sourcePath + " to " + destinationPath);
    }
    else
    {
        m_errorFlag = true;
        WriteToLog("Failed to move the file!");
        wxMessageBox("Failed to move the file!", "Error", wxOK | wxICON_ERROR);

    }
}

// Create a folder
void cMain::CreateFolder(const wxString& folderPath)
{
    if (wxMkdir(folderPath))
    {
        return;
    }
    else
    {
        m_errorFlag = true;
        WriteToLog("Failed to create the folder!");
        wxMessageBox("Failed to create the folder!", "Error", wxOK | wxICON_ERROR);
    }
}

// Create a file
void cMain::CreateFiles(const wxString& filePath)
{
    std::ofstream file(filePath.ToStdString(), std::ios::out);
    if (file.is_open())
    {
        file.close();
        return;
    }
    else
    {
        m_errorFlag = true;
        WriteToLog("Failed to create the file!");
        wxMessageBox("Failed to create the file!", "Error", wxOK | wxICON_ERROR);
    }
}

wxString GetUserPathVariable(const wxString& varName)
{
    wxRegKey regKey(wxRegKey::HKCU, "Environment");
    wxString userPathValue;

    if (regKey.Exists())
    {
        if (regKey.HasValue(varName))
        {
            regKey.QueryValue(varName, userPathValue);
        }
        else
        {
            wxMessageBox("The specified environment variable does not exist.", "Info", wxOK | wxICON_INFORMATION);
        }
    }
    else
    {
        wxMessageBox("Failed to access user environment variables.", "Error", wxOK | wxICON_ERROR);
    }

    return userPathValue;
}
wxString GetSystemPathVariable(const wxString& varName)
{
    // Access the SYSTEM environment variables registry location
    wxRegKey regKey(wxRegKey::HKLM, "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment");
    wxString systemPathValue;

    if (regKey.Exists())
    {
        if (regKey.HasValue(varName))
        {
            regKey.QueryValue(varName, systemPathValue);
        }
        else
        {
            wxMessageBox("The specified system environment variable does not exist.", "Info", wxOK | wxICON_INFORMATION);
        }
    }
    else
    {
        wxMessageBox("Failed to access system environment variables.", "Error", wxOK | wxICON_ERROR);
    }

    return systemPathValue;
}


// Set or add an environment variable
void cMain::SetEV(const wxString& varName, const wxString& varValue, bool append)
{
    if (varName.IsEmpty() || varValue.IsEmpty())
    {
        m_errorFlag = true;
        WriteToLog("Environment variable name or value cannot be empty!");
        wxMessageBox("Environment variable name or value cannot be empty!", "Error", wxOK | wxICON_ERROR);
        return;
    }

    wxString command;
    wxString currentValue;
    wxArrayString output;

    if (append)
    {
        // Retrieve the existing value using wxGetEnv (for current session)
        if (wxGetEnv(varName, &currentValue))
        {
            currentValue = currentValue.Trim().Trim(false);  // Trim leading/trailing spaces
        }
        else
        {
            currentValue = "";  // If variable doesn't exist, start with empty value
        }

        // Append the new value, preventing duplicate entries
        if (!currentValue.IsEmpty() && !currentValue.Contains(varValue))
        {
            currentValue += ";";
        }
        currentValue += varValue;

        command = "setx " + varName + " \"" + currentValue + "\"";
    }
    else
    {
        // Replace the variable value
        command = "setx " + varName + " \"" + varValue + "\"";
    }

    // Execute the command and check if successful
    int result = wxExecute(command, wxEXEC_SYNC);
    if (result == 0)
    {
        wxMessageBox("User environment variable updated successfully.\nYou may need to restart your session.",
            "Success", wxOK | wxICON_INFORMATION);
    }
    else
    {
        m_errorFlag = true;
        WriteToLog("Failed to update the environment variable.");
        wxMessageBox("Failed to update the environment variable.", "Error", wxOK | wxICON_ERROR);
    }
}
void cMain::SetSystemEV(const wxString& varName, const wxString& varValue, bool append)
{
    if (varName.IsEmpty() || varValue.IsEmpty())
    {
        m_errorFlag = true;
        WriteToLog("System environment variable name or value cannot be empty!");
        wxMessageBox("System environment variable name or value cannot be empty!", "Error", wxOK | wxICON_ERROR);
        return;
    }

    wxString command;
    wxArrayString output;

    if (append)
    {
        // Retrieve existing value using command prompt (system-level variable)
        wxExecute("cmd /c echo %" + varName + "%", output);
        wxString existingValue = output.IsEmpty() ? "" : output[0];

        // Append the new value
        wxString updatedValue = existingValue + ";" + varValue;
        command = "setx " + varName + " \"" + updatedValue + "\" /M";
    }
    else
    {
        // Replace the system environment variable
        command = "setx " + varName + " \"" + varValue + "\" /M";
    }

    wxExecute(command);
    wxMessageBox("System environment variable updated successfully.", "Success", wxOK | wxICON_INFORMATION);
}

void cMain::OnListBoxDoubleClick(wxCommandEvent& evt)
{
    // Get the index of the selected item
    int selectedIndex = m_listBox->GetSelection();
    if (selectedIndex == wxNOT_FOUND) return; // No item is selected

    // Get the current value of the selected item
    wxString currentValue = m_listBox->GetString(selectedIndex);

    wxDialog editDialog(this, wxID_ANY, "Edit List Item", wxDefaultPosition, wxSize(400, 250));

    // Create a vertical sizer for layout
    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

    wxChoice* choiceDropdown = new wxChoice(&editDialog, wxID_ANY, wxDefaultPosition, wxSize(300, 30), choices);
    vbox->Add(new wxStaticText(&editDialog, wxID_ANY, "Choose an option:"), 0, wxALL, 10);
    vbox->Add(choiceDropdown, 0, wxALL | wxEXPAND, 10);

    // Add text control for manual editing
    wxTextCtrl* textCtrl = new wxTextCtrl(&editDialog, wxID_ANY, currentValue, wxDefaultPosition, wxSize(300, 30));
    vbox->Add(new wxStaticText(&editDialog, wxID_ANY, "Or edit manually:"), 0, wxALL, 10);
    vbox->Add(textCtrl, 0, wxALL | wxEXPAND, 10);

    // Add OK and Cancel buttons
    wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
    wxButton* btnOk = new wxButton(&editDialog, wxID_OK, "OK");
    wxButton* btnCancel = new wxButton(&editDialog, wxID_CANCEL, "Cancel");
    hbox->Add(btnOk, 1, wxRIGHT, 10);
    hbox->Add(btnCancel, 1, wxRIGHT, 10);
    vbox->Add(hbox, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 15);

    editDialog.SetSizerAndFit(vbox);

    if (editDialog.ShowModal() == wxID_OK)
    {
        // Get the user's choice from the dropdown
        wxString selectedOption = choiceDropdown->GetStringSelection();
        wxString editedValue = textCtrl->GetValue().Trim();

        // Handle the selected option
        if (!selectedOption.IsEmpty())
        {
            if (selectedOption == "Run an Exe")
            {
                wxFileDialog openFileDialog(
                    this,
                    "Select an .exe file",
                    wxEmptyString,
                    "",
                    "Executable files (*.exe)|*.exe",
                    wxFD_OPEN | wxFD_FILE_MUST_EXIST);

                if (openFileDialog.ShowModal() == wxID_OK)
                {
                    wxString filePath = openFileDialog.GetPath();
                    m_listBox->SetString(selectedIndex, "Run .exe: " + filePath);
                }
            }
            else if (selectedOption == "Insert File")
            {
                wxFileDialog sourceFileDialog(
                    this,
                    "Select the file to insert",
                    wxEmptyString,
                    "",
                    "All files (*.*)|*.*",
                    wxFD_OPEN | wxFD_FILE_MUST_EXIST);

                if (sourceFileDialog.ShowModal() == wxID_OK)
                {
                    wxString sourcePath = sourceFileDialog.GetPath();
                    wxFileName sourceFile(sourcePath);
                    wxString fileName = sourceFile.GetFullName();

                    wxDirDialog destinationDirDialog(
                        this,
                        "Select where to insert file",
                        "",
                        wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

                    if (destinationDirDialog.ShowModal() == wxID_OK)
                    {
                        wxString destinationPath = destinationDirDialog.GetPath();
                        wxFileName destinationFile(destinationPath, fileName);
                        m_listBox->SetString(selectedIndex, "Move: \"" + sourcePath + "\" \"" + destinationFile.GetFullPath() + "\"");
                    }
                }
            }
            else if (selectedOption == "Create Folder")
            {
                wxDirDialog folderDialog(
                    this,
                    "Select the base folder where the new folder will be created:",
                    "",
                    wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

                if (folderDialog.ShowModal() == wxID_OK)
                {
                    wxString basePath = folderDialog.GetPath();

                    wxTextEntryDialog nameDialog(
                        this,
                        "Enter the name of the new folder:",
                        "New Folder Name",
                        "",
                        wxOK | wxCANCEL);

                    if (nameDialog.ShowModal() == wxID_OK)
                    {
                        wxString folderName = nameDialog.GetValue().Trim();
                        if (!folderName.IsEmpty())
                        {
                            wxString fullFolderPath = basePath + "\\" + folderName;
                            m_listBox->SetString(selectedIndex, "Create Folder: " + fullFolderPath);
                        }
                        else
                        {
                            wxMessageBox("Folder name cannot be empty!", "Error", wxOK | wxICON_ERROR);
                        }
                    }
                }
            }
            else if (selectedOption == "Create File")
            {
                wxFileDialog fileDialog(
                    this,
                    "Select a file to create:",
                    wxEmptyString,
                    "",
                    "All files (*.*)|*.*",
                    wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

                if (fileDialog.ShowModal() == wxID_OK)
                {
                    wxString filePath = fileDialog.GetPath();
                    m_listBox->SetString(selectedIndex, "Create File: " + filePath);
                }
            }
            else if (selectedOption == "Add/Edit Environment Variables")
            {
                wxTextEntryDialog varNameDialog(
                    this,
                    "Enter the name of the environment variable (e.g., PATH):",
                    "Add/Edit Environment Variable",
                    "",
                    wxOK | wxCANCEL);

                if (varNameDialog.ShowModal() == wxID_OK)
                {
                    wxString varName = varNameDialog.GetValue().Trim();
                    if (!varName.IsEmpty())
                    {
                        wxString currentValue = GetUserPathVariable(varName);
                        if (!currentValue.IsEmpty())
                        {
                            wxMessageDialog modifyDialog(
                                this,
                                "The variable already exists. Would you like to add to it or replace it?\n"
                                "Click Yes to Add, No to Replace.",
                                "Modify Environment Variable",
                                wxYES_NO | wxCANCEL | wxICON_QUESTION);

                            int modifyChoice = modifyDialog.ShowModal();
                            if (modifyChoice == wxID_YES)
                            {
                                wxTextEntryDialog varValueDialog(
                                    this,
                                    "Enter the value to add to the variable:",
                                    "Add to Environment Variable",
                                    "",
                                    wxOK | wxCANCEL);

                                if (varValueDialog.ShowModal() == wxID_OK)
                                {
                                    wxString newValue = varValueDialog.GetValue().Trim();
                                    if (!newValue.IsEmpty())
                                    {
                                        wxString updatedValue = currentValue + ";" + newValue;
                                        m_listBox->SetString(selectedIndex, "Set Environment Variable (Add): " + varName + "=" + updatedValue);
                                    }
                                }
                            }
                            else if (modifyChoice == wxID_NO)
                            {
                                wxTextEntryDialog varValueDialog(
                                    this,
                                    "Enter the new value for the variable:",
                                    "Replace Environment Variable",
                                    "",
                                    wxOK | wxCANCEL);

                                if (varValueDialog.ShowModal() == wxID_OK)
                                {
                                    wxString newValue = varValueDialog.GetValue().Trim();
                                    if (!newValue.IsEmpty())
                                    {
                                        m_listBox->SetString(selectedIndex, "Set Environment Variable (Replace): " + varName + "=" + newValue);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else if (selectedOption == "Add/Edit System Environment Variables")
            {
                wxTextEntryDialog varNameDialog(
                    this,
                    "Enter the name of the system environment variable (e.g., PATH):",
                    "Add/Edit System Environment Variable",
                    "",
                    wxOK | wxCANCEL);

                if (varNameDialog.ShowModal() == wxID_OK)
                {
                    wxString varName = varNameDialog.GetValue().Trim();
                    if (!varName.IsEmpty())
                    {
                        wxString currentValue = GetSystemPathVariable(varName);
                        if (!currentValue.IsEmpty())
                        {
                            wxMessageDialog modifyDialog(
                                this,
                                "The system variable already exists. Would you like to add to it or replace it?\n"
                                "Click Yes to Add, No to Replace.",
                                "Modify System Environment Variable",
                                wxYES_NO | wxCANCEL | wxICON_QUESTION);

                            int modifyChoice = modifyDialog.ShowModal();
                            if (modifyChoice == wxID_YES)
                            {
                                wxTextEntryDialog varValueDialog(
                                    this,
                                    "Enter the value to add to the system variable:",
                                    "Add to System Environment Variable",
                                    "",
                                    wxOK | wxCANCEL);

                                if (varValueDialog.ShowModal() == wxID_OK)
                                {
                                    wxString newValue = varValueDialog.GetValue().Trim();
                                    if (!newValue.IsEmpty())
                                    {
                                        wxString updatedValue = currentValue + ";" + newValue;
                                        m_listBox->SetString(selectedIndex, "Set System Environment Variable (Add): " + varName + "=" + updatedValue);
                                    }
                                }
                            }
                            else if (modifyChoice == wxID_NO)
                            {
                                wxTextEntryDialog varValueDialog(
                                    this,
                                    "Enter the new value for the system variable:",
                                    "Replace System Environment Variable",
                                    "",
                                    wxOK | wxCANCEL);

                                if (varValueDialog.ShowModal() == wxID_OK)
                                {
                                    wxString newValue = varValueDialog.GetValue().Trim();
                                    if (!newValue.IsEmpty())
                                    {
                                        m_listBox->SetString(selectedIndex, "Set System Environment Variable (Replace): " + varName + "=" + newValue);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else if (selectedOption == "Checkpoint")
            {
                wxTextEntryDialog nameDialog(
                    this,
                    "Enter Checkpoint Message",
                    "--CHECKPOINT--",
                    "",
                    wxOK | wxCANCEL);

                if (nameDialog.ShowModal() == wxID_OK)
                {
                    wxString message = nameDialog.GetValue().Trim();
                    if (!message.IsEmpty())
                    {
                        m_listBox->SetString(selectedIndex, "CHECKPOINT: " + message);
                    }
                }
            }
        }
        else
        {
            // Update with manually entered value
            if (!editedValue.IsEmpty())
            {
                m_listBox->SetString(selectedIndex, editedValue);
            }
        }
    }
}


void cMain::OnDeleteButtonClicked(wxCommandEvent& evt)
{
    // Get the currently selected item index
    int selection = m_listBox->GetSelection();

    if (selection != wxNOT_FOUND) // Ensure an item is selected
    {
        // Remove the selected item
        m_listBox->Delete(selection);
    }
    else
    {
        // If no item is selected, show a message box
        wxMessageBox("Please select an item to delete.", "No Selection", wxOK | wxICON_WARNING);
    }

    evt.Skip();
}

void cMain::OnButtonClicked(wxCommandEvent& evt)
{
    // Get the standard paths to create the results folder
    wxString resultsPath = wxStandardPaths::Get().GetExecutablePath();
    wxFileName exePath(resultsPath);
    wxString resultsDir = exePath.GetPath() + "\\results\\";

    // Get the selected option from the dropdown menu
    wxString selectedOption = m_choice->GetStringSelection();

    // If "Option 1" is selected, open a file dialog to select an .exe file
    if (selectedOption == "Run an Exe")
    {
        wxFileDialog openFileDialog(
            this,
            "Select an .exe file",
            resultsDir,  // Set the initial path to the results folder
            "",
            "Executable files (*.exe)|*.exe",
            wxFD_OPEN | wxFD_FILE_MUST_EXIST);

        // If the user selects a file, store its path
        if (openFileDialog.ShowModal() == wxID_OK)
        {
            wxString filePath = openFileDialog.GetPath();

            // Save the file path (you can use it elsewhere in your program)
            m_selectedFilePath = filePath;

            // Add the file path to the list box
            m_listBox->AppendString("Run .exe: " + filePath);
        }
    }
    else if (selectedOption == "Insert File")
    {
        // File dialog for selecting the source file
        wxFileDialog sourceFileDialog(
            this,
            "Select the file to insert",
            resultsDir,  // Set the initial path to the results folder
            "",
            "All files (*.*)|*.*",
            wxFD_OPEN | wxFD_FILE_MUST_EXIST);

        if (sourceFileDialog.ShowModal() == wxID_OK)
        {
            wxString sourcePath = sourceFileDialog.GetPath();

            // Extract the file name from the source path
            wxFileName sourceFile(sourcePath);
            wxString fileName = sourceFile.GetFullName();  // This gives just the file name (including extension)

            // File dialog for selecting the destination folder
            wxDirDialog destinationDirDialog(
                this,
                "Select where to insert file",
                "",
                wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

            if (destinationDirDialog.ShowModal() == wxID_OK)
            {
                wxString destinationPath = destinationDirDialog.GetPath();

                // Append the file name to the destination path
                wxFileName destinationFile(destinationPath, fileName);  // Combine folder and file name

                // Append the move command to the list box
                m_listBox->AppendString("Move: \"" + sourcePath + "\" \"" + destinationFile.GetFullPath() + "\"");
            }
        }
    }
    else if (selectedOption == "Create Folder")
    {
        // Step 1: Select the base folder path
        wxDirDialog folderDialog(
            this,
            "Select the base folder where the new folder will be created:",
            resultsDir,  // Set the initial path to the results folder
            wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

        if (folderDialog.ShowModal() == wxID_OK)
        {
            wxString basePath = folderDialog.GetPath(); // Get the base folder path

            // Step 2: Ask for the new folder name
            wxTextEntryDialog nameDialog(
                this,
                "Enter the name of the new folder:",
                "New Folder Name",
                "",
                wxOK | wxCANCEL);

            if (nameDialog.ShowModal() == wxID_OK)
            {
                wxString folderName = nameDialog.GetValue().Trim(); // Get the folder name

                // Ensure the folder name is valid
                if (!folderName.IsEmpty())
                {
                    // Construct the full folder path
                    wxString fullFolderPath = basePath + "\\" + folderName;

                    // Add to the list box
                    m_listBox->AppendString("Create Folder: " + fullFolderPath);
                }
                else
                {
                    wxMessageBox("Folder name cannot be empty!", "Error", wxOK | wxICON_ERROR);
                }
            }
        }
    }
    else if (selectedOption == "Create File")
    {
        // Dialog for specifying the file name and path
        wxFileDialog fileDialog(
            this,
            "Select a file to create:",
            resultsDir,  // Set the initial path to the results folder
            "",
            "All files (*.*)|*.*",
            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

        if (fileDialog.ShowModal() == wxID_OK)
        {
            wxString filePath = fileDialog.GetPath();
            m_listBox->AppendString("Create File: " + filePath);
        }
    }
    else if (selectedOption == "Add/Edit Environment Variables")
    {
        // Step 1: Input the name of the environment variable
        wxTextEntryDialog varNameDialog(
            this,
            "Enter the name of the environment variable (e.g., PATH):",
            "Add/Edit Environment Variable",
            "",
            wxOK | wxCANCEL);

        if (varNameDialog.ShowModal() == wxID_OK)
        {
            wxString varName = varNameDialog.GetValue().Trim();

            // Ensure the variable name is valid
            if (!varName.IsEmpty())
            {
                wxString currentValue = GetUserPathVariable(varName);
                if (!currentValue.IsEmpty())
                {
                    // Variable exists, ask if the user wants to add or edit
                    wxMessageDialog modifyDialog(
                        this,
                        "The variable already exists. Would you like to add to it or replace it?\n"
                        "Click Yes to Add, No to Replace.",
                        "Modify Environment Variable",
                        wxYES_NO | wxCANCEL | wxICON_QUESTION);

                    int modifyChoice = modifyDialog.ShowModal();

                    if (modifyChoice == wxID_YES)
                    {
                        // Add to the variable
                        wxTextEntryDialog varValueDialog(
                            this,
                            "Enter the value to add to the variable:",
                            "Add to Environment Variable",
                            "",
                            wxOK | wxCANCEL);

                        if (varValueDialog.ShowModal() == wxID_OK)
                        {
                            wxString newValue = varValueDialog.GetValue().Trim();

                            if (!newValue.IsEmpty())
                            {
                                // Append the new value to the existing variable
                                wxString updatedValue = currentValue + newValue + ";";
                                m_listBox->AppendString("Set Environment Variable (Add): " + varName + "=" + updatedValue);
                            }
                            else
                            {
                                wxMessageBox("Variable value cannot be empty!", "Error", wxOK | wxICON_ERROR);
                            }
                        }
                    }
                    else if (modifyChoice == wxID_NO)
                    {
                        // Replace the variable
                        wxTextEntryDialog varValueDialog(
                            this,
                            "Enter the new value for the variable:",
                            "Replace Environment Variable",
                            "",
                            wxOK | wxCANCEL);

                        if (varValueDialog.ShowModal() == wxID_OK)
                        {
                            wxString newValue = varValueDialog.GetValue().Trim();

                            if (!newValue.IsEmpty())
                            {
                                m_listBox->AppendString("Set Environment Variable (Replace): " + varName + "=" + newValue);
                            }
                            else
                            {
                                wxMessageBox("Variable value cannot be empty!", "Error", wxOK | wxICON_ERROR);
                            }
                        }
                    }
                }
                else
                {
                    // Variable doesn't exist, create a new one
                    wxTextEntryDialog varValueDialog(
                        this,
                        "The variable does not exist. Enter the value to create it:",
                        "Create Environment Variable",
                        "",
                        wxOK | wxCANCEL);

                    if (varValueDialog.ShowModal() == wxID_OK)
                    {
                        wxString newValue = varValueDialog.GetValue().Trim();

                        if (!newValue.IsEmpty())
                        {
                            m_listBox->AppendString("Set Environment Variable (Create): " + varName + "=" + newValue);
                        }
                        else
                        {
                            wxMessageBox("Variable value cannot be empty!", "Error", wxOK | wxICON_ERROR);
                        }
                    }
                }
            }
            else
            {
                wxMessageBox("Variable name cannot be empty!", "Error", wxOK | wxICON_ERROR);
            }
        }
    }
    else if (selectedOption == "Add/Edit System Environment Variables")
    {
        wxTextEntryDialog varNameDialog(
            this,
            "Enter the name of the system environment variable (e.g., PATH):",
            "Add/Edit System Environment Variable",
            "",
            wxOK | wxCANCEL);

        if (varNameDialog.ShowModal() == wxID_OK)
        {
            wxString varName = varNameDialog.GetValue().Trim();
            if (!varName.IsEmpty())
            {
                wxString currentValue = GetSystemPathVariable(varName);
                if (!currentValue.IsEmpty())
                {
                    wxMessageDialog modifyDialog(
                        this,
                        "The system variable already exists. Would you like to add to it or replace it?\n"
                        "Click Yes to Add, No to Replace.",
                        "Modify System Environment Variable",
                        wxYES_NO | wxCANCEL | wxICON_QUESTION);

                    int modifyChoice = modifyDialog.ShowModal();
                    if (modifyChoice == wxID_YES)
                    {
                        wxTextEntryDialog varValueDialog(
                            this,
                            "Enter the value to add to the system variable:",
                            "Add to System Environment Variable",
                            "",
                            wxOK | wxCANCEL);

                        if (varValueDialog.ShowModal() == wxID_OK)
                        {
                            wxString newValue = varValueDialog.GetValue().Trim();
                            if (!newValue.IsEmpty())
                            {
                                wxString updatedValue = currentValue + ";" + newValue;
                                m_listBox->AppendString("Set System Environment Variable (Add): " + varName + "=" + updatedValue);
                            }
                        }
                    }
                    else if (modifyChoice == wxID_NO)
                    {
                        wxTextEntryDialog varValueDialog(
                            this,
                            "Enter the new value for the system variable:",
                            "Replace System Environment Variable",
                            "",
                            wxOK | wxCANCEL);

                        if (varValueDialog.ShowModal() == wxID_OK)
                        {
                            wxString newValue = varValueDialog.GetValue().Trim();
                            if (!newValue.IsEmpty())
                            {
                                m_listBox->AppendString("Set System Environment Variable (Replace): " + varName + "=" + newValue);
                            }
                        }
                    }
                }
            }
        }
    }
    else if (selectedOption == "Checkpoint")
    {
        wxTextEntryDialog nameDialog(
            this,
            "Enter Checkpoint Message",
            "CHECKPOINT--",
            "",
            wxOK | wxCANCEL);

        if (nameDialog.ShowModal() == wxID_OK)
        {
            wxString message = nameDialog.GetValue().Trim(); // Get the mesage

            // Ensure the folder name is valid
            if (!message.IsEmpty())
            {

                // Add to the list box
                m_listBox->AppendString("CHECKPOINT: " + message);
            }
            else
            {
                wxMessageBox("Checkpoint message cannot be empty!", "Error", wxOK | wxICON_ERROR);
            }
        }
    }
    else
    {
        // For other options, just add the option name to the list box
        if (!selectedOption.IsEmpty())
        {
            m_listBox->AppendString(selectedOption);
        }
    }

    evt.Skip();
}

void cMain::OnSaveListboxClicked(wxCommandEvent& evt)
{
    // Get the standard paths to create the results folder
    wxString resultsPath = wxStandardPaths::Get().GetExecutablePath();
    wxFileName exePath(resultsPath);
    wxString resultsDir = exePath.GetPath() + "\\results\\";

    // Create the results folder if it doesn't exist
    if (!wxDirExists(resultsDir))
    {
        if (!wxMkdir(resultsDir))
        {
            wxMessageBox("Failed to create the 'results' folder!", "Error", wxOK | wxICON_ERROR);
            return; // Stop if folder creation fails
        }
    }

    // Prompt the user to choose a name for the listbox file, with the initial directory set to the results folder
    wxString fileName = wxFileSelector("Save Listbox", resultsDir, "", "", "Text files (*.txt)|*.txt", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    // Check if the user selected a file name or canceled the dialog
    if (fileName.IsEmpty())
    {
        return;  // User canceled the file selection, so do nothing
    }

    // Ensure the file ends with .txt extension if not already present
    if (!fileName.EndsWith(".txt"))
    {
        fileName += ".txt";
    }

    wxFileName filePath(fileName);
    wxString baseName = filePath.GetName();  // Extract "test" from "test.txt"
    wxString subDirPath = resultsDir + baseName + "\\";

    // Create the subfolder if it doesn't exist
    if (!wxDirExists(subDirPath))
    {
        if (!wxMkdir(subDirPath))
        {
            wxMessageBox("Failed to create the folder: " + subDirPath, "Error", wxOK | wxICON_ERROR);
            return;  // Stop if folder creation fails
        }
    }
    // Set the full path to save the file inside the subfolder
    wxString finalFilePath = subDirPath + filePath.GetFullName();

    // Open file for writing
    std::ofstream listboxFile(finalFilePath.ToStdString(), std::ios::out);

    // Check if file opened successfully
    if (!listboxFile.is_open())
    {
        // Print the actual file path to the message box for better debugging
        wxMessageBox("Failed to create the listbox file at: " + finalFilePath, "Error", wxOK | wxICON_ERROR);
        return;
    }

    // Write each item in the listbox to the file
    for (unsigned int i = 0; i < m_listBox->GetCount(); i++)
    {
        wxString item = m_listBox->GetString(i);
        listboxFile << item.ToStdString() << std::endl;
    }

    listboxFile.close();

    // Inform the user that the listbox has been saved
    wxMessageBox("Listbox saved to: " + finalFilePath, "Success", wxOK | wxICON_INFORMATION);
}

void cMain::OnOpenListboxClicked(wxCommandEvent& evt)
{
    // Get the standard paths to the results folder
    wxString resultsPath = wxStandardPaths::Get().GetExecutablePath();
    wxFileName exePath(resultsPath);
    wxString resultsDir = exePath.GetPath() + "\\results\\";

    // Prompt the user to open an existing listbox file, with the initial directory set to the results folder
    wxString fileName = wxFileSelector("Open Listbox", resultsDir, "", "", "Text files (*.txt)|*.txt", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    // Check if the user selected a file name or canceled the dialog
    if (fileName.IsEmpty())
    {
        wxMessageBox("No file selected or user canceled.", "Error", wxOK | wxICON_ERROR);
        return;  // User canceled the file selection, so do nothing
    }

    wxFileName scriptFile(fileName);
    m_scriptDirectory = scriptFile.GetPath();

    // Open the file for reading
    std::ifstream listboxFile(fileName.ToStdString(), std::ios::in);

    // Check if file opened successfully
    if (!listboxFile.is_open())
    {
        wxMessageBox("Failed to open the listbox file.", "Error", wxOK | wxICON_ERROR);
        return;
    }

    // Clear the current listbox before loading new items
    m_listBox->Clear();

    // Read each line from the file and add it to the listbox
    std::string line;
    while (std::getline(listboxFile, line))
    {
        m_listBox->AppendString(line);  // Convert std::string to wxString
    }

    listboxFile.close();

    // Inform the user that the listbox has been loaded
    // wxMessageBox("Listbox loaded from: " + fileName, "Success", wxOK | wxICON_INFORMATION);
}

void cMain::OnRunScriptClicked(wxCommandEvent& evt)
{
    // Check if the listbox is empty
    if (m_listBox->GetCount() == 0)
    {
        // Prompt the user to open a listbox file
        wxMessageBox("No steps are listed. Please open a listbox file.", "Error", wxOK | wxICON_ERROR);

        // Get the standard paths to create the results folder
        wxString resultsPath = wxStandardPaths::Get().GetExecutablePath();
        wxFileName exePath(resultsPath);
        wxString resultsDir = exePath.GetPath() + "\\results\\";

        // Open file dialog to load the listbox file, with the initial directory set to the results folder
        wxFileDialog openFileDialog(
            this,
            "Open Step File",
            resultsDir,  // Set the initial path to the results folder
            "",
            "Text files (*.txt)|*.txt",
            wxFD_OPEN | wxFD_FILE_MUST_EXIST);

        if (openFileDialog.ShowModal() == wxID_OK)
        {
            wxString filePath = openFileDialog.GetPath();
            std::ifstream file(filePath.ToStdString());

            if (file.is_open())
            {
                // Clear the current items in the listbox
                m_listBox->Clear();

                // Read the lines from the file and add them to the listbox
                std::string line;
                while (std::getline(file, line))
                {
                    m_listBox->AppendString(line);
                }
                file.close();
            }
            else
            {
                wxMessageBox("Failed to open the file.", "Error", wxOK | wxICON_ERROR);
            }
        }

        return;
    }
    int validItemCount = 0;
    for (unsigned int i = 0; i < m_listBox->GetCount(); i++)
    {
        wxString item = m_listBox->GetString(i).Trim();
        if (!item.IsEmpty())  // Count only non-empty items
        {
            validItemCount++;
        }
    }
    m_progressBar->SetRange(validItemCount);

    WriteToLog("Script started successfully.");

    wxString errorMarker = "Temp Restart Error Point (do not edit)";


    for (unsigned int i = 0; i < m_listBox->GetCount(); i++)
    {
        if (m_listBox->GetString(i) == errorMarker)
        {
            m_tempErrorPointIndex = i;
            break;
        }
    }

    // Determine the starting point: use the error point if it exists, else start from the beginning
    unsigned int startIndex = (m_tempErrorPointIndex != -1) ? m_tempErrorPointIndex : 0;

    // Iterate over the items in the list box and execute actions directly
    for (unsigned int i = startIndex; i < m_listBox->GetCount(); i++)
    {
        try
        {
            wxString item = m_listBox->GetString(i);

            // Update the progress bar
            m_progressBar->SetValue(i + 1);
            if (item.StartsWith("Run .exe: "))
            {
                // Extract the path after "Run .exe: "
                wxString exePath = item.Mid(10).Trim();
                wxString logMessage = wxString::Format("Performing operation: %d, Run .exe for path: %s", i, exePath);
                WriteToLog(logMessage);
                RunExe(exePath);  // Call the function to run the executable
                WriteToLog("Successfully ran .exe");
            }
            else if (item.StartsWith("Move: "))
            {
                // Step 1: Remove "Move: " part (6 characters)
                item = item.Mid(7);

                wxString logMessage = wxString::Format("Performing operation: %d, Move", i);
                WriteToLog(logMessage);

                // Step 2: Extract the first path
                wxString first_path = item.BeforeFirst('\"');  // Extract everything before the first quote

                // Step 3: Extract the second path
                wxString second_path = item.Mid(first_path.Len() + 3);  // Skip past the first quote and path

                second_path = second_path.BeforeFirst('\"');  // Extract everything before the next quote

                FileMover(first_path, second_path);
                WriteToLog("Successfully moved files");
            }
            else if (item.StartsWith("Create Folder: "))
            {
                // Extract folder path after "Create Folder: "
                wxString folderPath = item.Mid(15).Trim();

                wxString logMessage = wxString::Format("Performing operation: %d, Create Folder", i);
                WriteToLog(logMessage);

                CreateFolder(folderPath);  // Call the function to create the folder
                WriteToLog("Successfully created folder");
            }
            else if (item.StartsWith("Create File: "))
            {
                // Extract file path after "Create File: "
                wxString filePath = item.Mid(13).Trim();

                wxString logMessage = wxString::Format("Performing operation: %d, Create File", i);
                WriteToLog(logMessage);
                CreateFiles(filePath);  // Call the function to create the file
                WriteToLog("Successfully created file");
            }
            else if (item.StartsWith("Set Environment Variable (Add): "))
            {
                // Extract the environment variable name and value after "Set Environment Variable (Add): "
                wxString command = item.Mid(32).Trim();
                wxString logMessage = wxString::Format("Performing operation: %d, Set Environment Variable (Add)", i);
                WriteToLog(logMessage);

                size_t eqPos = command.Find('=');
                if (eqPos != wxNOT_FOUND)
                {
                    wxString varName = command.SubString(0, eqPos - 1);
                    wxString varValue = command.SubString(eqPos + 1, command.Length() - 1);
                    SetEV(varName, varValue, true);  // Call to append to the environment variable
                    WriteToLog("Successfully Set Environment Variable (Add)");
                }
            }
            else if (item.StartsWith("Set Environment Variable (Replace): "))
            {
                // Extract the environment variable name and value after "Set Environment Variable (Replace): "
                wxString command = item.Mid(36).Trim();
                wxString logMessage = wxString::Format("Performing operation: %d, Set Environment Variable (Replace)", i);
                WriteToLog(logMessage);
                size_t eqPos = command.Find('=');
                if (eqPos != wxNOT_FOUND)
                {
                    wxString varName = command.SubString(0, eqPos - 1);
                    wxString varValue = command.SubString(eqPos + 1, command.Length() - 1);
                    SetEV(varName, varValue, false);  // Call to replace the environment variable
                    WriteToLog("Successfully Set Environment Variable (Replace)");
                }
            }
            else if (item.StartsWith("Set Environment Variable (Create): "))
            {
                // Extract the environment variable name and value after "Set Environment Variable (Create): "
                wxString command = item.Mid(35).Trim();
                wxString logMessage = wxString::Format("Performing operation: %d, Set Environment Variable (Create)", i);
                WriteToLog(logMessage);
                size_t eqPos = command.Find('=');
                if (eqPos != wxNOT_FOUND)
                {
                    wxString varName = command.SubString(0, eqPos - 1);
                    wxString varValue = command.SubString(eqPos + 1, command.Length() - 1);
                    SetEV(varName, varValue, true);  // Call to create the environment variable
                    WriteToLog("Successfully Set Environment Variable (Create)");
                }
            }
            else if (item.StartsWith("Set System Environment Variable (Add): "))
            {
                // Extract the environment variable name and value after "Set System Environment Variable (Add): "
                wxString command = item.Mid(39).Trim();  // Length of the prefix "Set System Environment Variable (Add): "
                wxString logMessage = wxString::Format("Performing operation: %d, Set System Environment Variable (Add)", i);
                WriteToLog(logMessage);
                size_t eqPos = command.Find('=');
                if (eqPos != wxNOT_FOUND)
                {
                    wxString varName = command.SubString(0, eqPos - 1);
                    wxString varValue = command.SubString(eqPos + 1, command.Length() - 1);

                    // Call the function to append the system environment variable (true for append)
                    SetSystemEV(varName, varValue, true);
                    WriteToLog("Successfully Set System Environment Variable (Add)");
                }
            }
            else if (item.StartsWith("Set System Environment Variable (Replace): "))
            {
                // Extract the environment variable name and value after "Set System Environment Variable (Replace): "
                wxString command = item.Mid(43).Trim();  // Length of the prefix "Set System Environment Variable (Replace): "
                wxString logMessage = wxString::Format("Performing operation: %d, Set System Environment Variable (Replace)", i);
                WriteToLog(logMessage);
                size_t eqPos = command.Find('=');
                if (eqPos != wxNOT_FOUND)
                {
                    wxString varName = command.SubString(0, eqPos - 1);
                    wxString varValue = command.SubString(eqPos + 1, command.Length() - 1);

                    // Call the function to replace the system environment variable (false for replace)
                    SetSystemEV(varName, varValue, false);
                    WriteToLog("Successfully Set System Environment Variable (Replace)");
                }
            }
            else if (item.StartsWith("CHECKPOINT: "))
            {
                // Extract the checkpoint message after "CHECKPOINT: "
                wxString message = item.Mid(12);
                wxString logMessage = wxString::Format("Performing operation: %d, CHECKPOINT", i);
                WriteToLog(logMessage);

                // Show a message dialog with a "Next" button to pause execution
                wxDialog* checkpointDialog = new wxDialog(
                    this, wxID_ANY, "Checkpoint", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

                // Add a vertical box sizer for layout
                wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

                // Add the message as a static text
                wxStaticText* messageText = new wxStaticText(checkpointDialog, wxID_ANY, message);
                vbox->Add(messageText, 1, wxALL | wxEXPAND, 10);

                // Add a "Next" button
                wxButton* nextButton = new wxButton(checkpointDialog, wxID_OK, "Next");
                vbox->Add(nextButton, 0, wxALIGN_CENTER | wxALL, 10);

                // Set the sizer for the dialog
                checkpointDialog->SetSizerAndFit(vbox);
                checkpointDialog->SetSize(wxSize(300, 200));

                // Show the dialog and wait for the user to click "OK" (Next)
                checkpointDialog->ShowModal();
                delete checkpointDialog;  // Cleanup the dialog
                WriteToLog("Successfully did CHECKPOINT");
            }
            else if (item == errorMarker)
            {
                // Remove the "Temp Error Point" since we're resuming from it
                m_listBox->Delete(i);
                WriteToLog("Resuming from Temp Error Point at index: " + wxString::Format("%d", i));
                m_tempErrorPointIndex = -1;  // Reset the error point
                m_errorFlag = false;
                i--;
                continue;
            }
            else
            {
                if (!item.IsEmpty()) {
                    wxString logMessage = wxString::Format("Unknown command on step: %d, Name: %s", i, item);
                    WriteToLog(logMessage);

                    // Error
                    m_errorFlag = true;
                }

            }

            if (m_errorFlag)
            {
                WriteToLog("Encountered an error, adding Temp Error Point...");
                m_listBox->Insert(errorMarker, i);
                m_tempErrorPointIndex = i;
                return;  // Stop processing and wait for the next call
            }
        }
        catch (const std::exception& e)
        {
            // Log the error to file
            wxString logMessage = wxString::Format("Error processing item: %d, Error: %s", i, e.what());
            WriteToLog(logMessage);

            wxMessageBox(wxString::Format("Error processing item %d: %s", i, e.what()), "Error", wxOK | wxICON_ERROR);

            if (m_listBox->FindString(errorMarker) == wxNOT_FOUND)
            {
                m_listBox->Insert(errorMarker, i);
                m_tempErrorPointIndex = i;
                m_errorFlag = true;
            }
            break;
        }
    }

    if (!m_errorFlag)
    {
        m_tempErrorPointIndex = -1;
        WriteToLog("Script ended successfully.");
    }
    else
    {
        WriteToLog("Script ended with errors. Restart required.");
    }

    // Inform the user that the operations were executed
    // wxMessageBox("Operations executed successfully!", "Success", wxOK | wxICON_INFORMATION);
}


/*void cMain::OnSaveScriptClicked(wxCommandEvent& evt)
{
    // Get the custom script name from the text box
    wxString scriptName = m_txtScriptName->GetValue().Trim();
    if (scriptName.IsEmpty())
    {
        wxMessageBox("Please enter a name for the script!", "Error", wxOK | wxICON_ERROR);
        return;
    }

    // Add the .bat extension if not already present
    if (!scriptName.EndsWith(".bat"))
    {
        scriptName += ".bat";
    }

    // Get the standard paths to create the results folder
    wxString resultsPath = wxStandardPaths::Get().GetExecutablePath();
    wxFileName exePath(resultsPath);
    wxString resultsDir = exePath.GetPath() + "\\results\\";

    // Create the results folder if it doesn't exist
    if (!wxDirExists(resultsDir))
    {
        wxMkdir(resultsDir);
    }

    // Script file path
    wxString scriptPath = resultsDir + scriptName;

    // Open file for writing
    std::ofstream scriptFile(scriptPath.ToStdString());
    if (!scriptFile.is_open())
    {
        wxMessageBox("Failed to create the script file!", "Error", wxOK | wxICON_ERROR);
        return;
    }

    // Write each item in the list box to the script file
    for (unsigned int i = 0; i < m_listBox->GetCount(); i++)
    {
        wxString item = m_listBox->GetString(i);

        // Check if the item starts with "Run .exe: " and extract the path
        if (item.StartsWith("Run .exe: "))
        {
            wxString exePath = item.Mid(10);  // Extract the path after "Run .exe: "
            scriptFile << "\"" << exePath.ToStdString() << "\"" << std::endl;
        }
        else if (item.StartsWith("Move: "))
        {
            wxString moveCommand = item.Mid(6).Trim();  // Extract the move command after "Move: "
            scriptFile << "move " << moveCommand.ToStdString() << std::endl;
        }
        else if (item.StartsWith("Create Folder: "))
        {
            wxString folderPath = item.Mid(15).Trim();  // Extract folder path
            scriptFile << "mkdir \"" << folderPath.ToStdString() << "\"" << std::endl;
        }
        else if (item.StartsWith("Create File: "))
        {
            wxString filePath = item.Mid(13).Trim();  // Extract file path
            scriptFile << "type nul > \"" << filePath.ToStdString() << "\"" << std::endl;
        }
        else if (item.StartsWith("Set Environment Variable (Add): "))
        {
            wxString command = item.Mid(32).Trim();  // Extract the variable name and value
            size_t eqPos = command.Find('=');
            if (eqPos != wxNOT_FOUND)
            {
                wxString varName = command.SubString(0, eqPos - 1);
                wxString varValue = command.SubString(eqPos + 1, command.Length() - 1);
                scriptFile << "setx " << varName.ToStdString() << " \"" << varValue.ToStdString() << "\"" << std::endl;
            }
        }
        else if (item.StartsWith("Set Environment Variable (Replace): "))
        {
            wxString command = item.Mid(36).Trim();  // Extract the variable name and value
            size_t eqPos = command.Find('=');
            if (eqPos != wxNOT_FOUND)
            {
                wxString varName = command.SubString(0, eqPos - 1);
                wxString varValue = command.SubString(eqPos + 1, command.Length() - 1);
                scriptFile << "setx " << varName.ToStdString() << " \"" << varValue.ToStdString() << "\"" << std::endl;
            }
        }
        else if (item.StartsWith("Set Environment Variable (Create): "))
        {
            wxString command = item.Mid(35).Trim();  // Extract the variable name and value
            size_t eqPos = command.Find('=');
            if (eqPos != wxNOT_FOUND)
            {
                wxString varName = command.SubString(0, eqPos - 1);
                wxString varValue = command.SubString(eqPos + 1, command.Length() - 1);
                scriptFile << "setx " << varName.ToStdString() << " \"" << varValue.ToStdString() << "\"" << std::endl;
            }
        }
        else if (item.StartsWith("CHECKPOINT: "))
        {
            wxString message = item.Mid(14);  // Extract the path after "Run .exe: "
            scriptFile << "Pause. >nul | echo.  \"" << message.ToStdString() << "\"" << std::endl;
        }

    }

    scriptFile.close();

    // Inform the user that the script has been saved
    wxMessageBox("Script saved to: " + scriptPath, "Success", wxOK | wxICON_INFORMATION);
}
*/


