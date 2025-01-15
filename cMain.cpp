#include "cMain.h"
#include <wx/stdpaths.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/process.h>
#include <Windows.h>
#include <wx/dir.h>
#include <fstream>

wxBEGIN_EVENT_TABLE(cMain, wxFrame)
	EVT_BUTTON(1001, OnButtonClicked)
    EVT_BUTTON(1002, OnRunScriptClicked)
    EVT_BUTTON(1003, OnSaveListboxClicked)
    EVT_BUTTON(1004, OnOpenListboxClicked)
wxEND_EVENT_TABLE()

// Run an Executable (.exe)
void cMain::RunExe(const wxString& exePath)
{
    // Execute the file directly
    int retCode = wxExecute(exePath, wxEXEC_SYNC);
    if (retCode == -1)
    {
        wxMessageBox("Failed to execute the program!", "Error", wxOK | wxICON_ERROR);
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
        wxMessageBox("Failed to move the file!", "Error", wxOK | wxICON_ERROR);
    }
}

// Create a folder
void cMain::CreateFolder(const wxString& folderPath)
{
    if (wxMkdir(folderPath))
    {
        m_listBox->AppendString("Created Folder: " + folderPath);
    }
    else
    {
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
        m_listBox->AppendString("Created File: " + filePath);
    }
    else
    {
        wxMessageBox("Failed to create the file!", "Error", wxOK | wxICON_ERROR);
    }
}

// Set or add an environment variable
void cMain::SetEV(const wxString& varName, const wxString& varValue, bool append)
{
    if (varName.IsEmpty() || varValue.IsEmpty())
    {
        wxMessageBox("Environment variable name or value cannot be empty!", "Error", wxOK | wxICON_ERROR);
        return;
    }

    // Windows-specific code
#if defined(_WIN32) || defined(_WIN64)
    if (append)
    {
        // Try to get the current value of the environment variable
        wxString currentValue;
        if (wxGetEnv(varName, &currentValue))
        {
            // Append the new value to the current value
            currentValue += ";" + varValue;

            // Set the new value for the environment variable
            if (!SetEnvironmentVariable(varName, currentValue))
            {
                wxMessageBox("Failed to append environment variable.", "Error", wxOK | wxICON_ERROR);
            }
        }
        else
        {
            // If the variable doesn't exist, just set it to the new value
            if (!SetEnvironmentVariable(varName, varValue))
            {
                wxMessageBox("Failed to set environment variable.", "Error", wxOK | wxICON_ERROR);
            }
        }
    }
    else
    {
        // Set the environment variable (this will overwrite it if it already exists)
        if (!SetEnvironmentVariable(varName, varValue))
        {
            wxMessageBox("Failed to set environment variable.", "Error", wxOK | wxICON_ERROR);
        }
    }
#endif
}

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
    choices.Add("Checkpoint");
    m_choice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(150, 30), choices);
    vbox->Add(m_choice, 0, wxALIGN_LEFT | wxTOP, 10);  // Center horizontally, 10px from the top

    // "+" Button to add the selected option to the list
    m_btnAdd = new wxButton(this, 1001, "+", wxDefaultPosition, wxSize(50, 30));
    vbox->Add(m_btnAdd, 0, wxALIGN_LEFT | wxTOP, 0);  // Center horizontally, 0 px from the dropdown

    // Middle box (wxListBox) to display selected options
    m_listBox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(500, 300));
    vbox->Add(m_listBox, 1, wxALIGN_CENTER | wxALL, 10);  // Expandable and centered

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
    else if (selectedOption == "Move File")
    {
        // File dialog for selecting the source file
        wxFileDialog sourceFileDialog(
            this,
            "Select the file to move",
            resultsDir,  // Set the initial path to the results folder
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

    // Open file for writing
    std::ofstream listboxFile(fileName.ToStdString(), std::ios::out);

    // Check if file opened successfully
    if (!listboxFile.is_open())
    {
        // Print the actual file path to the message box for better debugging
        wxMessageBox("Failed to create the listbox file at: " + fileName, "Error", wxOK | wxICON_ERROR);
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
    wxMessageBox("Listbox saved to: " + fileName, "Success", wxOK | wxICON_INFORMATION);
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
    wxMessageBox("Listbox loaded from: " + fileName, "Success", wxOK | wxICON_INFORMATION);
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
    // Iterate over the items in the list box and execute actions directly
    for (unsigned int i = 0; i < m_listBox->GetCount(); i++)
    {
        wxString item = m_listBox->GetString(i);

        if (item.StartsWith("Run .exe: "))
        {
            // Extract the path after "Run .exe: "
            wxString exePath = item.Mid(10).Trim();
            RunExe(exePath);  // Call the function to run the executable
        }
        else if (item.StartsWith("Move: "))
        {
            // Extract the move command after "Move: "
            wxString moveCommand = item.Mid(6).Trim();
            size_t spacePos = moveCommand.Find(" ");
            if (spacePos != wxNOT_FOUND)
            {
                wxString sourcePath = moveCommand.SubString(0, spacePos - 1);
                wxString destinationPath = moveCommand.SubString(spacePos + 1, moveCommand.Length() - 1);
                FileMover(sourcePath, destinationPath);  // Call the function to move the file
            }
        }
        else if (item.StartsWith("Create Folder: "))
        {
            // Extract folder path after "Create Folder: "
            wxString folderPath = item.Mid(15).Trim();
            CreateFolder(folderPath);  // Call the function to create the folder
        }
        else if (item.StartsWith("Create File: "))
        {
            // Extract file path after "Create File: "
            wxString filePath = item.Mid(13).Trim();
            CreateFiles(filePath);  // Call the function to create the file
        }
        else if (item.StartsWith("Set Environment Variable (Add): "))
        {
            // Extract the environment variable name and value after "Set Environment Variable (Add): "
            wxString command = item.Mid(32).Trim();
            size_t eqPos = command.Find('=');
            if (eqPos != wxNOT_FOUND)
            {
                wxString varName = command.SubString(0, eqPos - 1);
                wxString varValue = command.SubString(eqPos + 1, command.Length() - 1);
                SetEV(varName, varValue, true);  // Call to append to the environment variable
            }
        }
        else if (item.StartsWith("Set Environment Variable (Replace): "))
        {
            // Extract the environment variable name and value after "Set Environment Variable (Replace): "
            wxString command = item.Mid(36).Trim();
            size_t eqPos = command.Find('=');
            if (eqPos != wxNOT_FOUND)
            {
                wxString varName = command.SubString(0, eqPos - 1);
                wxString varValue = command.SubString(eqPos + 1, command.Length() - 1);
                SetEV(varName, varValue, false);  // Call to replace the environment variable
            }
        }
        else if (item.StartsWith("Set Environment Variable (Create): "))
        {
            // Extract the environment variable name and value after "Set Environment Variable (Create): "
            wxString command = item.Mid(35).Trim();
            size_t eqPos = command.Find('=');
            if (eqPos != wxNOT_FOUND)
            {
                wxString varName = command.SubString(0, eqPos - 1);
                wxString varValue = command.SubString(eqPos + 1, command.Length() - 1);
                SetEV(varName, varValue, true);  // Call to create the environment variable
            }
        }
        else if (item.StartsWith("CHECKPOINT: "))
        {
            // Extract the checkpoint message after "CHECKPOINT: "
            wxString message = item.Mid(14);

            // Show a message dialog with a "Next" button to pause execution
            wxMessageDialog* checkpointDialog = new wxMessageDialog(
                this,
                message + "\nClick 'Next' to proceed.",
                "Checkpoint",
                wxOK | wxICON_INFORMATION
            );

            // Show the dialog and wait for the user to click "OK" (Next)
            checkpointDialog->ShowModal();
            delete checkpointDialog;  // Cleanup the dialog
        }
    }

    // Inform the user that the operations were executed
    wxMessageBox("Operations executed successfully!", "Success", wxOK | wxICON_INFORMATION);
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


