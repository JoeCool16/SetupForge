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
    choices.Add("Option 2");
    choices.Add("Option 3");
    choices.Add("Option 4");
    choices.Add("Option 5");
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
    }

    scriptFile.close();

    // Inform the user that the script has been saved
    wxMessageBox("Script saved to: " + scriptPath, "Success", wxOK | wxICON_INFORMATION);
}
