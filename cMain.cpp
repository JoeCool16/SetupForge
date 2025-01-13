#include "cMain.h"

wxBEGIN_EVENT_TABLE(cMain, wxFrame)
	EVT_BUTTON(1001, OnButtonClicked)
wxEND_EVENT_TABLE()

cMain::cMain() : wxFrame(nullptr, wxID_ANY, "Setup Forge", wxPoint(30, 30), wxSize(800, 600))
{
	//m_btn1 = new wxButton(this, 1001, "Click Me", wxPoint(10, 10), wxSize(150, 50));
	//m_txt1 = new wxTextCtrl(this, wxID_ANY, "", wxPoint(10, 70), wxSize(300, 30));
	//m_list1 = new wxListBox(this, wxID_ANY, wxPoint(10, 110), wxSize(300, 300));
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
    m_listBox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(300, 300));
    vbox->Add(m_listBox, 1, wxALIGN_CENTER | wxALL, 10);  // Expandable and centered

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
            m_listBox->AppendString("Selected .exe: " + filePath);
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
