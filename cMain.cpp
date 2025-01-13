#include "cMain.h"
#include <wx/stdpaths.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <fstream>

wxBEGIN_EVENT_TABLE(cMain, wxFrame)
	EVT_BUTTON(1001, OnButtonClicked)
    EVT_BUTTON(1002, OnSaveScriptClicked)
wxEND_EVENT_TABLE()

cMain::cMain() : wxFrame(nullptr, wxID_ANY, "Setup Forge", wxPoint(30, 30), wxSize(800, 600))
{
    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

    // Dropdown (wxChoice) to select options
    wxArrayString choices;
    choices.Add("Run an Exe");
    choices.Add("Move File");
    choices.Add("Create Folder");
    choices.Add("Create File");
    choices.Add("Add/Edit Environment Variables");
    m_choice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(150, 30), choices);
    vbox->Add(m_choice, 0, wxALIGN_LEFT | wxTOP, 10);  // Center horizontally, 10px from the top

    // "+" Button to add the selected option to the list
    m_btnAdd = new wxButton(this, 1001, "+", wxDefaultPosition, wxSize(50, 30));
    vbox->Add(m_btnAdd, 0, wxALIGN_LEFT | wxTOP, 0);  // Center horizontally, 0 px from the dropdown

    // Middle box (wxListBox) to display selected options
    m_listBox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(500, 300));
    vbox->Add(m_listBox, 1, wxALIGN_CENTER | wxALL, 10);  // Expandable and centered

    // Horizontal sizer for Save Script Button and Textbox
    wxBoxSizer* saveBox = new wxBoxSizer(wxHORIZONTAL);

    // Label for the text box
    wxStaticText* lblScriptName = new wxStaticText(this, wxID_ANY, "File Name:", wxDefaultPosition, wxDefaultSize);
    saveBox->Add(lblScriptName, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);  // Add label with spacing to the right

    // Textbox for custom script name
    m_txtScriptName = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(150, 30));
    saveBox->Add(m_txtScriptName, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

    // Save Script Button
    m_btnSave = new wxButton(this, 1002, "Save Script", wxDefaultPosition, wxSize(100, 30));
    saveBox->Add(m_btnSave, 0, wxALIGN_CENTER_VERTICAL);

    vbox->Add(saveBox, 0, wxALIGN_CENTER | wxTOP, 10);

    // Set the sizer for the frame
    this->SetSizer(vbox);

}

cMain::~cMain()
{
}

void cMain::OnButtonClicked(wxCommandEvent& evt)
{
    // Get the selected option from the dropdown menu
    wxString selectedOption = m_choice->GetStringSelection();

    // If "Option 1" is selected, open a file dialog to select an .exe file
    if (selectedOption == "Run an Exe")
    {
        wxFileDialog openFileDialog(
            this,
            "Select an .exe file",
            "", "",
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
    else if (selectedOption == "Move File")
    {
        // File dialog for selecting the source file
        wxFileDialog sourceFileDialog(
            this,
            "Select the file to move",
            "",
            "",
            "All files (*.*)|*.*",
            wxFD_OPEN | wxFD_FILE_MUST_EXIST);

        if (sourceFileDialog.ShowModal() == wxID_OK)
        {
            wxString sourcePath = sourceFileDialog.GetPath();

            // File dialog for selecting the destination folder
            wxDirDialog destinationDirDialog(
                this,
                "Select the destination folder",
                "",
                wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

            if (destinationDirDialog.ShowModal() == wxID_OK)
            {
                wxString destinationPath = destinationDirDialog.GetPath();

                // Append the move command to the list box
                m_listBox->AppendString("Move: \"" + sourcePath + "\" \"" + destinationPath + "\"");
            }
        }
    }
    else if (selectedOption == "Create Folder")
    {
        // Step 1: Select the base folder path
        wxDirDialog folderDialog(
            this,
            "Select the base folder where the new folder will be created:",
            "",
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
            "",
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
                // Check if the variable exists (user-level)
                wxString currentValue;
                if (wxGetEnv(varName, &currentValue))
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

void cMain::OnSaveScriptClicked(wxCommandEvent& evt)
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

    }

    scriptFile.close();

    // Inform the user that the script has been saved
    wxMessageBox("Script saved to: " + scriptPath, "Success", wxOK | wxICON_INFORMATION);
}
