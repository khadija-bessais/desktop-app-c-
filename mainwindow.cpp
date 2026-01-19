#include "mainwindow.h"
#include "ui_mainwindow.h"
using namespace qrcodegen;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    ,ui(new Ui::MainWindow)
    , c("", 0, "", "", "")  // Assuming 'e' is your equipment object
    , e(0, "", "", "", "", "", "")
    , currentImageIndex(0)  // Initialize currentImageIndex correctly here
{
    ui->setupUi(this);
    ui->client_tableView->setModel(c.afficher());
    // Connection co;
    // Connect the selectionChanged signal to the slot
    // Connect the clicked signal of the client_tableView to the slot
        connect(ui->client_tableView, &QTableView::clicked,this, &MainWindow::on_client_tableView_clicked);
        int rowHeight6 = 30; // Adjust this value to your desired row height
        ui->clients_calander_tab_view->verticalHeader()->setDefaultSectionSize(rowHeight6);

        int rowWidth6 = 220;
        ui->clients_calander_tab_view->horizontalHeader()->setDefaultSectionSize(rowWidth6);

        connect(ui->lineEditSearchClient, &QLineEdit::textChanged, this, &MainWindow::on_lineEditSearchClient_textChanged);

        loadChart("Reservations");


        // Connect the calendar selection changed signal to the slot
        connect(ui->clients_calendarWidget, &QCalendarWidget::selectionChanged,
                this, &MainWindow::on_clients_calendarWidget_selectionChanged);
        highlightReservationDates();
        ///arduino
            int ret=arduino.connect_arduino(); // lancer la connexion à arduino
            switch(ret){
            case(0):qDebug()<< "arduino is available and connected to : "<< arduino.getarduino_port_name();
                break;
            case(1):qDebug() << "arduino is available but not connected to :" <<arduino.getarduino_port_name();
               break;
            case(-1):qDebug() << "arduino is not available";
            }

            QObject::connect(arduino.getserial(),SIGNAL(readyRead()),this,SLOT(readFromArduino()));
            //equippement
            // Connect the button to the QR code generation slot
            //connect(ui->generateQRCodeButton, &QPushButton::clicked, this, &MainWindow::on_generateQRCodeButton_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//client
void MainWindow::on_client_add_btn_clicked()
{
    QRegularExpression idRegex("\\d{8}");
    QRegularExpression nomPrenomRegex("[a-zA-ZéèàêâûôùçÉÈÀÊÂÛÔÙÇ'-]+");
    QRegularExpression telephoneRegex("\\+216\\d{8}"); // Match "+216" followed by exactly 8 digits
    QRegularExpression sexeRegex("[malefemale]");

    QString idString = ui->client_lineEditCin->text();
    int cin = idString.toInt();
    QString nom = ui->client_lineEditNom->text();
    QString prenom = ui->client_lineEditPrenom->text();
    QString telephone = ui->client_lineEditTelephone->text();
    QString sexe = ui->client_comboBoxSexe->currentText();
    QDate resDate = ui->client_res_dateEdit->date(); // Retrieve the date from the date edit
    QDate tomorrow = QDate::currentDate().addDays(1); // Calculate tomorrow's date

    // New client starts with 1 reservation
    int nbRes = 1;

    if (idString.isEmpty() || nom.isEmpty() || prenom.isEmpty() || telephone.isEmpty() || sexe.isEmpty() || !resDate.isValid()) {
        QMessageBox::warning(this, "Champs requis", "Veuillez remplir tous les champs obligatoires.");
    } else if (!idRegex.match(idString).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "CIN invalide. Veuillez saisir un entier de 8 chiffres.");
    } else if (!nomPrenomRegex.match(nom).hasMatch() || !nomPrenomRegex.match(prenom).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "Nom ou prénom invalide. Veuillez saisir des lettres alphabétiques.");
    } else if (!telephoneRegex.match(telephone).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "Numéro de téléphone invalide. Le format requis est '+216 XXXXXXXXX'.");
    } else if (!sexeRegex.match(sexe).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "Sexe invalide. Veuillez saisir 'm' pour masculin ou 'f' pour féminin.");
    } else if (resDate < tomorrow) {
        QMessageBox::warning(this, "Erreur", "La date de réservation doit être demain ou ultérieure.");
    } else {
        // Create a new client object with the new variables
        Client newClient(nom, cin, prenom, telephone, sexe, resDate.toString("yyyy-MM-dd"), nbRes);

        if (newClient.ajouter()) {
            // Update the chart and table view
            loadChart("Reservations");
            ui->client_tableView->setModel(newClient.afficher());

            // Populate combo box with updated CIN values

            // Clear input fields
            ui->client_lineEditCin->clear();
            ui->client_lineEditNom->clear();
            ui->client_lineEditPrenom->clear();
            ui->client_lineEditTelephone->clear();
            ui->client_comboBoxSexe->setCurrentIndex(0);
            ui->client_res_dateEdit->setDate(QDate::currentDate().addDays(1)); // Reset to tomorrow
            highlightReservationDates();

            QMessageBox::information(this, "Succès", "Client ajouté avec succès !");
        } else {
            QMessageBox::warning(this, "Échec", "Échec de l'ajout du client.");
        }
    }
}


void MainWindow::on_client_update_btn_clicked()
{
    // Regular expressions for input validation
    QRegularExpression idRegex("\\d{8}");
    QRegularExpression nomPrenomRegex("[a-zA-ZéèàêâûôùçÉÈÀÊÂÛÔÙÇ'-]+");
    QRegularExpression telephoneRegex("\\+216\\d{8}"); // Match "+216" followed by exactly 8 digits
    QRegularExpression sexeRegex("[malefemale]");

    // Get all selected indexes (cells) and ensure at least one cell is selected
    QModelIndexList selectedIndexes = ui->client_tableView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, tr("Aucune sélection"), tr("Veuillez sélectionner une ou plusieurs cellules dans la même ligne à modifier."));
        return;
    }

    // Check that all selected cells belong to the same row
    int firstRow = selectedIndexes.at(0).row();
    for (const QModelIndex &index : selectedIndexes) {
        if (index.row() != firstRow) {
            QMessageBox::warning(this, "Erreur", "Veuillez sélectionner des attributs dans la même ligne uniquement.");
            return;
        }
    }

    // Retrieve CIN for updating and check input fields for changes
    int selectedRow = firstRow;
    int cin = ui->client_tableView->model()->index(selectedRow, 1).data().toInt(); // Assuming CIN is in the second column

    // Retrieve existing data for the selected row
    QString currentNom = ui->client_tableView->model()->index(selectedRow, 0).data().toString();
    QString currentTelephone = ui->client_tableView->model()->index(selectedRow, 2).data().toString();
    QString currentPrenom = ui->client_tableView->model()->index(selectedRow, 3).data().toString();
    QString currentSexe = ui->client_tableView->model()->index(selectedRow, 4).data().toString();
    QDate currentDate = ui->client_tableView->model()->index(selectedRow, 5).data().toDate(); // Assuming date is in the sixth column
    int currentNbRes = ui->client_tableView->model()->index(selectedRow, 6).data().toInt(); // Assuming nb_res_cl is in the seventh column

    // Get values from input fields, using current values if fields are empty
    QString nom = ui->client_lineEditNom->text().isEmpty() ? currentNom : ui->client_lineEditNom->text();
    QString telephone = ui->client_lineEditTelephone->text().isEmpty() ? currentTelephone : ui->client_lineEditTelephone->text();
    QString prenom = ui->client_lineEditPrenom->text().isEmpty() ? currentPrenom : ui->client_lineEditPrenom->text();
    QString sexe = ui->client_comboBoxSexe->currentText().isEmpty() ? currentSexe : ui->client_comboBoxSexe->currentText();
    QDate resDate = ui->client_res_dateEdit->date();

    QDate tomorrow = QDate::currentDate().addDays(1); // Calculate tomorrow's date

    // Validation checks for each field
    if (!idRegex.match(QString::number(cin)).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "CIN invalide. Veuillez saisir un entier de 8 chiffres.");
        return;
    }
    if (!nomPrenomRegex.match(nom).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "Nom invalide. Veuillez saisir des lettres alphabétiques.");
        return;
    }
    if (!nomPrenomRegex.match(prenom).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "Prénom invalide. Veuillez saisir des lettres alphabétiques.");
        return;
    }
    if (!telephoneRegex.match(telephone).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "Numéro de téléphone invalide. Le format requis est '+216 XXXXXXXXX'.");
        return;
    }
    if (!sexeRegex.match(sexe).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "Sexe invalide. Veuillez saisir 'm' pour masculin ou 'f' pour féminin.");
        return;
    }
    if (resDate < tomorrow) {
        QMessageBox::warning(this, "Erreur", "La date de réservation ne peut être qu'à partir de demain.");
        return;
    }

    // Check if the date has changed and increment nb_res_cl only if it has
    int updatedNbRes = currentNbRes;
    if (resDate != currentDate) {
        updatedNbRes++; // Increment nb_res_cl
    }

    // Only proceed with the update if there are changes
    if (nom != currentNom || telephone != currentTelephone || prenom != currentPrenom || sexe != currentSexe || resDate != currentDate) {
        // Call the Client::modifier function
        if (c.modifier(cin, nom, telephone, prenom, sexe, resDate.toString("yyyy-MM-dd"), updatedNbRes)) {
            // Refresh the table view
            ui->client_tableView->setModel(c.afficher());
           loadChart("Reservations");

            // Clear input fields
            ui->client_lineEditCin->clear();
            ui->client_lineEditNom->clear();
            ui->client_lineEditPrenom->clear();
            ui->client_lineEditTelephone->clear();
            ui->client_comboBoxSexe->setCurrentIndex(0);
            ui->client_res_dateEdit->setDate(QDate::currentDate().addDays(1)); // Reset to tomorrow
            highlightReservationDates();

            QMessageBox::information(this, "Succès", "Client mis à jour avec succès !");
        } else {
            QMessageBox::warning(this, "Échec", "Échec de la mise à jour du client.");
        }
    } else {
        QMessageBox::information(this, "Aucun changement", "Aucun changement détecté dans les champs.");
    }
}



void MainWindow::on_client_delete_btn_clicked()
{
    // Get all selected indexes (cells) and ensure at least one cell is selected
    QModelIndexList selectedIndexes = ui->client_tableView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, tr("Aucune sélection"), tr("Veuillez sélectionner une cellule ou une ligne à supprimer."));
        return;
    }

    // Check that all selected cells belong to the same row
    int firstRow = selectedIndexes.at(0).row();
    for (const QModelIndex &index : selectedIndexes) {
        if (index.row() != firstRow) {
            QMessageBox::warning(this, "Erreur", "Veuillez sélectionner des attributs dans la même ligne uniquement.");
            return;
        }
    }

    // Retrieve CIN from the selected row for deletion
    QString cin = ui->client_tableView->model()->index(firstRow, 1).data().toString(); // CIN_CL assumed to be in the second column

    // Confirm deletion with the user
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmer la suppression", "Êtes-vous sûr de vouloir supprimer ce client?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Attempt deletion
        if (c.supprimer(cin.toInt())) {
            // Update the table and other UI components
            loadChart("Reservations");
            ui->client_tableView->setModel(c.afficher());
            highlightReservationDates();
            QMessageBox::information(this, "Succès", "Client supprimé avec succès !");
        } else {
            QMessageBox::warning(this, "Échec", "Échec de la suppression du client.");
        }
    }
}


void MainWindow::on_client_tri_btn__clicked() {
    QSqlQuery query;

    // Retrieve the selected value from client_TriBox
    QString sortingCriteria = ui->client_TriBox->currentText().trimmed(); // Remove any trailing/leading whitespace for safety

    // Prepare the SQL query based on the selected criteria
    if (sortingCriteria == "nom_ascendant") {
        query.prepare("SELECT * FROM client ORDER BY NOM_CL ASC");
    } else if (sortingCriteria == "nom_descendant") {
        query.prepare("SELECT * FROM client ORDER BY NOM_CL DESC");
    } else if (sortingCriteria == "id_ascendant") {
        query.prepare("SELECT * FROM client ORDER BY CIN_CL ASC");
    } else if (sortingCriteria == "id_descendant") {
        query.prepare("SELECT * FROM client ORDER BY CIN_CL DESC");
    } else if (sortingCriteria == "sexe_ascendant") {
        // Sort females on top
        query.prepare("SELECT * FROM client ORDER BY CASE WHEN SEXE_CL = 'female' THEN 1 ELSE 2 END ASC, SEXE_CL ASC");
    } else if (sortingCriteria == "sexe_descendant") {
        // Sort males on top
        query.prepare("SELECT * FROM client ORDER BY CASE WHEN SEXE_CL = 'male' THEN 1 ELSE 2 END ASC, SEXE_CL ASC");
    } else {
        // Handle unexpected values
        QMessageBox::warning(this, "Erreur", "Critère de tri invalide !");
        return;
    }

    // Execute query and update the model
    if (query.exec()) {
        QSqlQueryModel *model = new QSqlQueryModel();
        model->setQuery(query);
        ui->client_tableView->setModel(model);
    } else {
        QMessageBox::warning(this, "Erreur", "Échec du tri des clients: " + query.lastError().text());
    }
}


void MainWindow::loadData(){
    QSqlQuery query;
    query.prepare("SELECT CIN_CL FROM client");
     ui->client_tableView->setModel(c.afficher());
     // Set the minimum date to today
     ui->client_res_dateEdit->setMinimumDate(QDate::currentDate());

     // Allow the user to choose any date within the specified range
     ui->client_res_dateEdit->setDate(QDate::currentDate()); // Default to today

      loadChart("Reservations");
      highlightReservationDates();
      //equip
      ui->tableViewEquipement->setModel(e.afficher());
      equip_loadChart("Type");
}

void MainWindow::on_lineEditSearchClient_textChanged(const QString &text)
{
    QString searchType = ui->client_searchBox->currentText(); // Get the selected search type from the combo box
    QSqlQuery query;

    if (text.isEmpty()) {
        // If the search field is empty, reset the table view to show all clients
        query.prepare("SELECT * FROM client");
    } else {
        // Determine which column to search based on the combo box value
        if (searchType == "Nom") {
            query.prepare("SELECT * FROM client WHERE NOM_CL LIKE :term");
        } else if (searchType == "Prénom") {
            query.prepare("SELECT * FROM client WHERE PRENOM_CL LIKE :term");
        } else if (searchType == "ID") {
            query.prepare("SELECT * FROM client WHERE CIN_CL LIKE :term");
        } else {
            QMessageBox::warning(this, "Erreur", "Critère de recherche invalide !");
            return;
        }
        query.bindValue(":term", text + "%"); // Matches rows where the column starts with the input text
    }

    // Execute the query and update the table view
    if (query.exec()) {
        QSqlQueryModel *model = new QSqlQueryModel();
        model->setQuery(query);
        ui->client_tableView->setModel(model); // Dynamically update the client_tableView
    } else {
        qDebug() << "Search query error:" << query.lastError().text();
    }
}


void MainWindow::on_client_exportPdf_btn_clicked()
{
    // Open the Save File dialog to get the output file name
    QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter en PDF"), "", tr("Fichiers PDF (*.pdf)"));

    if (fileName.isEmpty()) {
        return;  // Exit if no file is selected
    }

    // Prepare the HTML content for the PDF
    QString html;
    html += "<html><head><title>Liste des Clients</title>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; color: white; background-color: black; font-size: 12pt; text-align: center; margin: 0; padding: 0; }";
    html += "h1 { font-size: 16pt; color: #87CEEB; margin-bottom: 20px; text-align: center; }";  // Sky Blue for title
    html += "table { width: 80%; margin: 30px auto; border-collapse: collapse; }";
    html += "th, td { padding: 10px; text-align: left; border: 1px solid #ddd; font-size: 12pt; color: white; }";
    html += "th { background-color: #4CAF50; color: white; }";
    html += "tr:hover { background-color: #333; }";  // Darker hover effect for table rows
    html += "img.logo { position: absolute; top: 20px; right: 20px; width: 100px; height: auto; }";  // Logo position
    html += "</style>";
    html += "</head><body>";

    // Add the logo image to the top right corner
    html += "<img class='logo' src=':/ressources/logo/logo.png' />";

    // Add the title in the center
    html += "<h1>Liste des Clients</h1>";

    // Start the table
    html += "<table>";
    html += "<tr><th>CIN</th><th>Nom</th><th>Prénom</th><th>Téléphone</th><th>Sexe</th><th>Date de Réservation</th><th>Nb Réservations</th></tr>";

    // Retrieve data from the table model and add to the HTML content
    for (int row = 0; row < ui->client_tableView->model()->rowCount(); ++row) {
        QString cin = ui->client_tableView->model()->index(row, 1).data().toString(); // CIN (index 1)
        QString nom = ui->client_tableView->model()->index(row, 0).data().toString(); // Nom (index 0)
        QString prenom = ui->client_tableView->model()->index(row, 2).data().toString(); // Prenom (index 2)
        QString telephone = ui->client_tableView->model()->index(row, 3).data().toString(); // Telephone (index 3)
        QString sexe = ui->client_tableView->model()->index(row, 4).data().toString(); // Sexe (index 4)
        QString date = ui->client_tableView->model()->index(row, 5).data().toString(); // Date de Réservation (index 5)
        QString nb_res = ui->client_tableView->model()->index(row, 6).data().toString(); // Nb Réservations (index 6)

        html += "<tr><td>" + cin + "</td><td>" + nom + "</td><td>" + prenom + "</td><td>" + telephone
                + "</td><td>" + sexe + "</td><td>" + date + "</td><td>" + nb_res + "</td></tr>";
    }

    html += "</table></body></html>";

    // Create a QTextDocument to hold the HTML content
    QTextDocument document;
    document.setHtml(html);

    // Set up the printer for PDF output
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);

    // Print the document to the specified PDF file
    document.print(&printer);  // No need to check the return value

    // Notify user that the PDF export was successful
    QMessageBox::information(this, tr("Exportation réussie"), tr("La liste des clients a été exportée avec succès en PDF."));
}


void MainWindow::on_client_tableView_clicked(const QModelIndex &index)
{
    // Get the row from the clicked index
    int selectedRow = index.row();

    // Retrieve data from the selected row
    QString nom = ui->client_tableView->model()->index(selectedRow, 0).data().toString();        // NOM_CL
    QString cin = ui->client_tableView->model()->index(selectedRow, 1).data().toString();        // CIN_CL
    QString prenom = ui->client_tableView->model()->index(selectedRow, 2).data().toString();     // PRENOM_CL
    QString telephone = ui->client_tableView->model()->index(selectedRow, 3).data().toString();  // TELEPHONE_CL
    QString sexe = ui->client_tableView->model()->index(selectedRow, 4).data().toString();       // SEXE_CL
    QString res_date_cl = ui->client_tableView->model()->index(selectedRow, 5).data().toString(); // RES_DATE_CL

    // Convert the date string to QDate
    QDate reservationDate = QDate::fromString(res_date_cl, "yyyy-MM-dd");

    // Populate the UI elements with the selected row's data
    ui->client_lineEditNom->setText(nom);
    ui->client_lineEditCin->setText(cin);
    ui->client_lineEditPrenom->setText(prenom);
    ui->client_lineEditTelephone->setText(telephone);
    ui->client_comboBoxSexe->setCurrentText(sexe);
    ui->client_res_dateEdit->setDate(reservationDate);


}

//chart
void MainWindow::createPieChartReservations(QFrame *frame, const QString &title, QStringList reservationCategories, const QMap<QString, QList<int>> &data) {
    QPieSeries *series = new QPieSeries();

    // Calculate the total number of clients across all categories
    int totalClients = 0;
    for (const auto &categoryData : data) {
        totalClients += categoryData.size();
    }

    // Iterate through the reservation categories
    for (const QString &category : reservationCategories) {
        if (data.contains(category)) {
            const QList<int> &categoryData = data[category];
            int categoryCount = categoryData.size();

            // Calculate the percentage
            double percentage = (totalClients > 0) ? (static_cast<double>(categoryCount) / totalClients) * 100 : 0;

            // Manually create the label for the pie slice
            QString sliceLabel = category + ": " + QString::number(categoryCount) + " clients (" + QString::number(percentage, 'f', 2) + "%)";

            // Create the pie slice and append it to the series
            QPieSlice *slice = new QPieSlice(sliceLabel, categoryCount);
            slice->setLabel(sliceLabel);  // Set the label directly on the slice

            // Optionally, customize the slice color for better visibility
            if (category == "Low (1-5)") {
                slice->setBrush(Qt::green);
            } else if (category == "Medium (6-10)") {
                slice->setBrush(Qt::yellow);
            } else if (category == "High (11+)") {
                slice->setBrush(Qt::red);
            }

            series->append(slice);
        }
    }

    // Create the chart and add the series
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(title);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Create a chart view and add it to the frame
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // Clear the existing contents of the frame
    QLayout *layout = frame->layout();
    if (layout) {
        QLayoutItem *item;
        while ((item = layout->takeAt(0))) {
            delete item->widget();
            delete item;
        }
    }

    frame->layout()->addWidget(chartView);
}

void MainWindow::loadChart(QString chartType) {
    // Get the current date
    QDate currentDate = QDate::currentDate();
    QString currentDateStr = currentDate.toString("yyyy-MM-dd");

    if (chartType == "Reservations") {
        // Retrieve reservation data (categories based on the number of reservations)
        QMap<QString, QList<int>> reservationData = c.retrieveReservationData("client");

        // Define the reservation categories (Low, Medium, High)
        QStringList reservationCategories = {"Low (1-5)", "Medium (6-10)", "High (11+)"};

        // Create the pie chart with reservation data
        createPieChartReservations(ui->clientChartFrame, "Client Reservations Distribution (" + currentDateStr + ")", reservationCategories, reservationData);
    }
}



void MainWindow::on_client_sms_Btn_clicked()
{
    // Retrieve the client ID (CIN_CL) from the line edit
    QString clientID = ui->client_lineEditCin->text();

    if (clientID.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez entrer un ID (CIN_CL) pour envoyer le SMS.");
        return;
    }

    // Query the database to get the phone number and NB_RES_CL for the given CIN_CL
    QSqlQuery query;
    query.prepare("SELECT TELEPHONE_CL, NB_RES_CL FROM client WHERE CIN_CL = :cin");
    query.bindValue(":cin", clientID);

    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, "Erreur", "Client introuvable ou erreur lors de la récupération des données.");
        return;
    }

    // Extract the phone number and NB_RES_CL
    QString phoneNumber = query.value("TELEPHONE_CL").toString();
    int nbResCl = query.value("NB_RES_CL").toInt();

    // Validate the phone number format (+216xxxxxxxx)
    QRegularExpression telephoneRegex("\\+216\\d{8}");
    if (!telephoneRegex.match(phoneNumber).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "Numéro de téléphone invalide pour ce client.");
        return;
    }

    // Calculate the total points
    int totalPoints = nbResCl * 100;

    // Prepare the SMS message
    QString message = QString("Cher client, vous avez accumulé un total de %1 points. Merci pour votre fidélité !").arg(totalPoints);

    // Use the original sendSms function
    sms.sendSms(phoneNumber, message);

    // Inform the user that the SMS sending process has been initiated
    QMessageBox::information(this, "SMS Envoyé", "Le SMS est en cours d'envoi au numéro: " + phoneNumber);
}



//////////////////////calendar///////////////////////////////////////////////////////////////////////////////////////

void MainWindow::addDataToTableViewClientCalander(const QList<QMap<QString, QString>>& dataList)
{
    QStandardItemModel* model = new QStandardItemModel(this);
    QStringList headers = {"Nom", "Prenom", "CIN", "Telephone", "Sexe", "Date Reservation", "Nb Reservations"};
    model->setHorizontalHeaderLabels(headers);

    // Populate the model with data
    for (const auto& row : dataList) {
        QList<QStandardItem*> items;
        items << new QStandardItem(row.value("nom_cl"))
              << new QStandardItem(row.value("prenom_cl"))
              << new QStandardItem(row.value("cin_cl"))
              << new QStandardItem(row.value("telephone_cl"))
              << new QStandardItem(row.value("sexe_cl"))
              << new QStandardItem(row.value("res_date_cl"))
              << new QStandardItem(row.value("nb_res_cl"));
        model->appendRow(items);
    }

    // Set the model to the table view
    ui->clients_calander_tab_view->setModel(model);
}

void MainWindow::highlightReservationDates()
{
    // Get all reservation dates
    QList<QDate> reservationDates = c.retrieveReservationDates("client");

    // Create a QTextCharFormat for highlighting
    QTextCharFormat format;
    format.setBackground(Qt::yellow);  // Highlight with a yellow background
    format.setForeground(Qt::black);  // Set text color to black
    format.setFontWeight(QFont::Bold);

    // Apply formatting to the calendar widget
    for (const QDate& date : reservationDates) {
        ui->clients_calendarWidget->setDateTextFormat(date, format);
    }
}

void MainWindow::on_clients_calendarWidget_selectionChanged()
{
    // Get the selected date from the calendar widget
    QDate selectedDate = ui->clients_calendarWidget->selectedDate();

    // Display the selected date in the line edit
    ui->lineEdit_date->setText(selectedDate.toString("yyyy-MM-dd"));

    // Retrieve client data for the selected date
    QList<QMap<QString, QString>> dataList = c.retrieveAllCalanderDataAsList("client", selectedDate);

    // Populate the table view with the retrieved data
    addDataToTableViewClientCalander(dataList);
}

bool MainWindow::userExists(QString id_emp) {
    QSqlQuery query;

    // Verify that the query checks ID_EMP from the EMPLOYEE table
    query.prepare("SELECT COUNT(*) FROM EMPLOYEE WHERE ID_EMP = :id_emp");
    query.bindValue(":id_emp", id_emp);

    // Execute the query and verify its success
    if (!query.exec()) {
        qDebug() << "Error executing query:" << query.lastError().text();
        return false;
    }

    // Ensure the query retrieves the expected value
    if (query.next()) {
        int count = query.value(0).toInt();
        return (count > 0); // Return true if at least one record matches
    }

    return false;
}



void MainWindow::readFromArduino() {
    QList<QByteArray> messages = arduino.read_from_arduino();
    QString string_to_check;

    if (arduino.containsSubstring(messages, "id:")) {
        if (!messages.isEmpty()) {
            QByteArray lastItem = messages.last();
            string_to_check = QString::fromLatin1(lastItem).trimmed();
        }

        // Ensure 'id:' substring exists and proper slicing
        if (string_to_check.startsWith("id:") && string_to_check.length() >= 11) {  // 'id:' + 8 characters
            QString current_id = string_to_check.mid(3, 8).trimmed();  // Extract ID, trimmed for whitespace issues
            qDebug() << "Scanned ID:" << current_id;

            if (!current_id.isEmpty() && userExists(current_id)) {
                QString towrite = "op-dr:" + current_id + "*";
                qDebug() << "Sending command to Arduino:" << towrite;
                arduino.write_to_arduino(towrite.toUtf8());  // Open door command
                QMessageBox::information(this, "Succès", "Bienvenue ! ID: " + current_id);
            } else {
                QMessageBox::warning(this, "Refusé", "Accès refusé. Veuillez être un employé. ID: " + current_id);
                qDebug() << "Card declined: ID not found in database.";
            }
        } else {
            qDebug() << "Malformed ID message received:" << string_to_check;
        }
    } else {
        qDebug() << "No valid 'id:' message detected from Arduino.";
    }
}

//equipement

//equippement crud

void MainWindow::on_equip_add_btn_clicked()
{
    // Regular expressions for validation
    QRegularExpression refRegex("\\d+");                 // Only digits for reference
    QRegularExpression quantiteRegex("\\d+");            // Only digits for quantity
    QRegularExpression prixRegex("\\d+(\\.\\d{1,2})?");  // Decimal or integer for price
    QRegularExpression nomRegex("[a-zA-ZéèàêâûôùçÉÈÀÊÂÛÔÙÇ'\\-\\s]+");  // Alphabetic and spaces for name
    QRegularExpression marqueRegex("[a-zA-ZéèàêâûôùçÉÈÀÊÂÛÔÙÇ'\\-\\s]+"); // Alphabetic and spaces for marque (brand)
    QRegularExpression typeRegex("[a-zA-ZéèàêâûôùçÉÈÀÊÂÛÔÙÇ'\\-\\s]+");  // Alphabetic and spaces for type
    QRegularExpression etatRegex("[a-zA-ZéèàêâûôùçÉÈÀÊÂÛÔÙÇ'\\-\\s]+");  // Alphabetic and spaces for state


    // Fetch values from input fields
    QString refString = ui->lineEditReference->text().trimmed();
    QString quantite = ui->lineEditQuantite->text().trimmed();
    QString prix = ui->lineEditPrix->text().trimmed();
    QString nom = ui->lineEditNom->text().trimmed();
    QString marque = ui->lineEditMarque->text().trimmed();
    QString type = ui->comboBoxType->currentText().trimmed();
    QString etat = ui->comboBoxEtat->currentText().trimmed();


    // Validations with detailed messages
    if (refString.isEmpty()) {
        QMessageBox::warning(this, "Champ requis", "Veuillez saisir une référence.");
    } else if (!refRegex.match(refString).hasMatch()) {
        QMessageBox::warning(this, "Erreur de saisie", "Référence invalide. Elle doit être un entier.");
    } else if (quantite.isEmpty()) {
        QMessageBox::warning(this, "Champ requis", "Veuillez saisir la quantité.");
    } else if (!quantiteRegex.match(quantite).hasMatch()) {
        QMessageBox::warning(this, "Erreur de saisie", "Quantité invalide. Elle doit être un nombre entier.");
    } else if (prix.isEmpty()) {
        QMessageBox::warning(this, "Champ requis", "Veuillez saisir un prix.");
    } else if (!prixRegex.match(prix).hasMatch()) {
        QMessageBox::warning(this, "Erreur de saisie", "Prix invalide. Il doit être un nombre entier ou décimal.");
    } else if (nom.isEmpty()) {
        QMessageBox::warning(this, "Champ requis", "Veuillez saisir le nom de l'équipement.");
    } else if (!nomRegex.match(nom).hasMatch()) {
        QMessageBox::warning(this, "Erreur de saisie", "Nom invalide. Il doit être composé de caractères alphabétiques.");
    } else if (marque.isEmpty()) {
        QMessageBox::warning(this, "Champ requis", "Veuillez saisir la marque.");
    } else if (!marqueRegex.match(marque).hasMatch()) {
        QMessageBox::warning(this, "Erreur de saisie", "Marque invalide. Elle doit être composée de caractères alphabétiques.");
    } else if (type.isEmpty()) {
        QMessageBox::warning(this, "Champ requis", "Veuillez saisir le type.");
    } else if (!typeRegex.match(type).hasMatch()) {
        QMessageBox::warning(this, "Erreur de saisie", "Type invalide. Il doit être composé de caractères alphabétiques.");
    } else if (etat.isEmpty()) {
        QMessageBox::warning(this, "Champ requis", "Veuillez saisir l'état.");
    } else if (!etatRegex.match(etat).hasMatch()) {
        QMessageBox::warning(this, "Erreur de saisie", "État invalide. Il doit être composé de caractères alphabétiques.");
    } else {
        // If all validations pass, create and add the new Equipement
        int reference = refString.toInt();
        Equipement newEquipement(reference, quantite, prix, nom, marque, type, etat);

        if (newEquipement.ajouter()) {
            // Refresh the table view (assuming afficher() returns a model)
            ui->tableViewEquipement->setModel(newEquipement.afficher());

            // Clear input fields
            ui->lineEditReference->clear();
            ui->lineEditQuantite->clear();
            ui->lineEditPrix->clear();
            ui->lineEditNom->clear();
            ui->lineEditMarque->clear();
            ui->comboBoxType->setCurrentIndex(0);
            ui->comboBoxEtat->setCurrentIndex(0);
            equip_loadChart("Type");

            QMessageBox::information(this, "Succès", "Équipement ajouté avec succès !");
        } else {
            QMessageBox::warning(this, "Échec", "Échec de l'ajout de l'équipement.");
        }
    }

}


void MainWindow::on_equip_update_btn_clicked()
{
    // Regular expressions for input validation
    QRegularExpression refRegex("^\\d+$");                   // Only digits for reference
    QRegularExpression quantiteRegex("^\\d+$");              // Only digits for quantity
    QRegularExpression prixRegex("^\\d+(\\.\\d{1,2})?$");    // Decimal or integer for price
    QRegularExpression nomRegex("^[a-zA-ZéèàêâûôùçÉÈÀÊÂÛÔÙÇ'\\-\\s]+$");  // Alphabetic and spaces for name
    QRegularExpression marqueRegex("^[a-zA-ZéèàêâûôùçÉÈÀÊÂÛÔÙÇ'\\-\\s]+$"); // Alphabetic and spaces for brand
    QRegularExpression typeRegex("^[a-zA-ZéèàêâûôùçÉÈÀÊÂÛÔÙÇ'\\-\\s]+$");  // Alphabetic and spaces for type
    QRegularExpression etatRegex("^[a-zA-ZéèàêâûôùçÉÈÀÊÂÛÔÙÇ'\\-\\s]+$");  // Alphabetic and spaces for state

    // Check if no row is selected
    if (!ui->tableViewEquipement->selectionModel()->hasSelection()) {
        QMessageBox::warning(this, tr("Aucune sélection"), tr("Veuillez sélectionner une ligne à modifier."));
        return;
    }


    QString referenceEq=QString::number(temp_eq.getreference());

    // Get the new values from the line edits or combo boxes

    QString nomEq = ui->lineEditNom->text();
    QString quantiteEq = ui->lineEditQuantite->text();
    QString prixEq = ui->lineEditPrix->text();
    QString marqueEq = ui->lineEditMarque->text();
    QString typeEq = ui->comboBoxType->currentText();
    QString etatEq = ui->comboBoxEtat->currentText();

    // Validation checks for the new values (only validate fields that are non-empty)
    if (!refRegex.match(referenceEq).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "Référence invalide. Veuillez saisir des chiffres.");
        return;
    }

    if (!nomEq.isEmpty() && !nomRegex.match(nomEq).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "Nom invalide. Veuillez saisir des lettres alphabétiques.");
        return;
    }
    if (!quantiteEq.isEmpty() && !quantiteRegex.match(quantiteEq).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "Quantité invalide. Veuillez saisir des chiffres.");
        return;
    }
    if (!prixEq.isEmpty() && !prixRegex.match(prixEq).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "Prix invalide. Veuillez saisir un nombre valide (ex: 100 ou 100.50).");
        return;
    }
    if (!marqueEq.isEmpty() && !marqueRegex.match(marqueEq).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "Marque invalide. Veuillez saisir des lettres alphabétiques.");
        return;
    }
    if (!typeEq.isEmpty() && !typeRegex.match(typeEq).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "Type invalide. Veuillez saisir des lettres alphabétiques.");
        return;
    }
    if (!etatEq.isEmpty() && !etatRegex.match(etatEq).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "État invalide. Veuillez saisir des lettres alphabétiques.");
        return;
    }

    // Prepare the SQL update statement
    QSqlQuery query;
    query.prepare("UPDATE equipement SET NOM_EQ = :nom, QUANTITE_EQ = :quantite, PRIX_EQ = :prix, "
                  "MARQUE_EQ = :marque, TYPE_EQ = :type, ETAT_EQ = :etat WHERE REFERENCE_EQ = :reference");

    // Bind all values directly
    query.bindValue(":nom", nomEq);
    query.bindValue(":quantite", quantiteEq.toInt());
    query.bindValue(":prix", prixEq.toDouble());
    query.bindValue(":marque", marqueEq);
    query.bindValue(":type", typeEq);
    query.bindValue(":etat", etatEq);
    query.bindValue(":reference", referenceEq.toInt());

    // Execute the update query and check for success
    if (query.exec()) {
        // Refresh the table view
        ui->tableViewEquipement->setModel(e.afficher());
        // Clear input fields
        ui->lineEditReference->clear();
        ui->lineEditQuantite->clear();
        ui->lineEditPrix->clear();
        ui->lineEditNom->clear();
        ui->lineEditMarque->clear();
        ui->comboBoxType->setCurrentIndex(0);
        ui->comboBoxEtat->setCurrentIndex(0);
        temp_eq.setreference(0);
        equip_loadChart("Type");
        QMessageBox::information(this, "Succès", "Équipement mis à jour avec succès !");
    } else {
        QMessageBox::warning(this, "Échec", "Échec de la mise à jour de l'équipement : " + query.lastError().text());
    }

}


void MainWindow::on_equip_delete_btn_clicked()
{
    if (!ui->tableViewEquipement->selectionModel()->hasSelection()) {
        QMessageBox::warning(this, tr("No Selection"), tr("Please select a row to delete."));
        return;
    }

    QModelIndexList selectedRows = ui->tableViewEquipement->selectionModel()->selectedRows();
    int selectedRow = selectedRows.at(0).row();
    int reference = ui->tableViewEquipement->model()->index(selectedRow, 0).data().toInt(); // REFERENCE_EQ from the table

    if (e.supprimer(reference)) {
        ui->tableViewEquipement->setModel(e.afficher());
        equip_loadChart("Type");
        QMessageBox::information(this, "Success", "Equipment deleted successfully!");
    } else {
        QMessageBox::warning(this, "Failure", "Failed to delete equipment.");
    }

}


void MainWindow::on_equip_exportPdf_btn_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter en PDF"), "", tr("Fichiers PDF (*.pdf)"));

    if (fileName.isEmpty()) {
        return;  // Quit if no file is selected
    }

    // Prepare HTML content for the PDF
    QString html;
    html += "<html><head><title>Liste des Equipements</title>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; color: #333; }";
    html += "h1 { text-align: center; color: #4CAF50; }";
    html += "table { width: 100%; border-collapse: collapse; margin: 20px 0; }";
    html += "th, td { padding: 10px; text-align: left; border: 1px solid #ddd; }";
    html += "th { background-color: #4CAF50; color: white; }";
    html += "tr:hover { background-color: #f1f1f1; }";
    html += "</style>";
    html += "</head><body>";
    html += "<h1>Liste des Equipements</h1>";
    html += "<table>";
    html += "<tr><th>Référence</th><th>Nom</th><th>Marque</th><th>Quantité</th><th>Prix</th><th>Type</th><th>État</th></tr>";

    // Retrieve the list of equipment from the QSqlQueryModel
    QSqlQueryModel *model = e.afficher(); // Use the afficher() function of Equipement

    // Iterate through the rows of the model and write to the HTML
    for (int row = 0; row < model->rowCount(); ++row) {
        int reference = model->index(row, 0).data().toInt();
        QString nom = model->index(row, 1).data().toString();
        QString marque = model->index(row, 2).data().toString();
        QString quantite = model->index(row, 3).data().toString();
        QString prix = model->index(row, 4).data().toString();
        QString type = model->index(row, 5).data().toString();
        QString etat = model->index(row, 6).data().toString();

        html += "<tr><td>" + QString::number(reference) + "</td><td>" + nom + "</td><td>" + marque + "</td><td>"
                + quantite + "</td><td>" + prix + "</td><td>" + type + "</td><td>" + etat + "</td></tr>";
    }

    html += "</table></body></html>";

    // Create a document for rendering as PDF
    QTextDocument document;
    document.setHtml(html);

    // Configure the PDF file output using QPrinter
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);

    // Print the document to the PDF
    document.print(&printer);

    QMessageBox::information(this, tr("Exportation réussie"), tr("La liste des équipements a été exportée avec succès en PDF."));

}







void MainWindow::on_tri_btn_clicked() {
    // Toggle sorting order
    sortAscending = !sortAscending;

    // Get sorting criteria from combo boxes
    QString sortingCriteria = ui->comboBoxTri1->currentText();
    QString sortingOrder = ui->comboBoxTri2->currentText();

    // Mapping for database column names
    QMap<QString, QString> attributeMap = {
        {"Reference", "REFERENCE_EQ"},
        {"Quantite", "QUANTITE_EQ"},
        {"Prix", "PRIX_EQ"},
        {"Nom", "NOM_EQ"},
        {"Marque", "MARQUE_EQ"},
        {"Type", "TYPE_EQ"},
        {"Etat", "ETAT_EQ"}
    };

    if (!attributeMap.contains(sortingCriteria)) {
        QMessageBox::warning(this, "Erreur", "Critère de tri invalide !");
        return;
    }

    QString columnName = attributeMap[sortingCriteria];
    QString order = (sortingOrder == "Ascendant") ? "ASC" : "DESC";

    // Prepare the SQL query
    QSqlQuery query;
    query.prepare(QString("SELECT * FROM equipement ORDER BY %1 %2")
                      .arg(columnName)
                      .arg(order));

    // Execute query and update the model
    if (query.exec()) {
        QSqlQueryModel *model = new QSqlQueryModel();
        model->setQuery(query);
        ui->tableViewEquipement->setModel(model);
    } else {
        QMessageBox::warning(this, "Erreur", "Échec du tri des équipements: " + query.lastError().text());
    }
}



void MainWindow::on_search_btn_clicked()
{
    // Retrieve search text from the line edit and selected attribute from the combo box
    QString searchText = ui->lineEditSearchReference->text();
    QString selectedAttribute = ui->comboBoxSearch->currentText();

    if (searchText.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a value to search.");
        return;
    }

    // Prepare the query dynamically based on the selected attribute
    QSqlQuery query;
    if (selectedAttribute == "Reference") {
        // Check if the search text is numeric
        bool isNumeric;
        int referenceValue = searchText.toInt(&isNumeric);

        if (!isNumeric) {
            QMessageBox::warning(this, "Error", "Please enter a valid numeric reference.");
            return;
        }
        query.prepare("SELECT * FROM equipement WHERE REFERENCE_EQ = :value");
        query.bindValue(":value", referenceValue);
    } else if (selectedAttribute == "Marque") {
        // Search by Marque (string)
        query.prepare("SELECT * FROM equipement WHERE MARQUE_EQ LIKE :value");
        query.bindValue(":value", "%" + searchText + "%");
    } else if (selectedAttribute == "Nom") {
        // Search by Nom (string)
        query.prepare("SELECT * FROM equipement WHERE NOM_EQ LIKE :value");
        query.bindValue(":value", "%" + searchText + "%");
    } else {
        // For other attributes, allow partial matches
        query.prepare(QString("SELECT * FROM equipement WHERE %1 LIKE :value").arg(selectedAttribute));
        query.bindValue(":value", "%" + searchText + "%");
    }

    // Execute the query and check for results
    if (query.exec()) {
        // If results exist, highlight the row
        if (query.next()) {
            // Equipment found
            QMessageBox::information(this, "Equipment Found", "Equipment found in the database.");

            // Ensure the model is updated
            QSqlQueryModel* model = new QSqlQueryModel();  // Create a new model to refresh the table
            model->setQuery(query);  // Set the result of the query to the model
            ui->tableViewEquipement->setModel(model);  // Set the model to the table view

            // Find the correct column index based on the selected attribute
            int targetColumn = -1;
            for (int col = 0; col < model->columnCount(); ++col) {
                if (model->headerData(col, Qt::Horizontal).toString().compare(selectedAttribute, Qt::CaseInsensitive) == 0) {
                    targetColumn = col;
                    break;
                }
            }

            if (targetColumn == -1) {
                //QMessageBox::warning(this, "Error", "Attribute not found in the table model.");
                return;
            }

            // Find and select the matching row in the table view
            for (int row = 0; row < model->rowCount(); ++row) {
                QModelIndex index = model->index(row, targetColumn);
                if (index.data().toString().contains(searchText, Qt::CaseInsensitive)) {
                    ui->tableViewEquipement->selectRow(row);
                    return; // Select the first matching row
                }
            }
        } else {
            // Equipment not found
            QMessageBox::warning(this, "Not Found", "No equipment matches this search.");
        }
    } else {
        QMessageBox::warning(this, "Error", "Failed to execute the query.");
    }
}

void MainWindow::on_lineEditSearchReference_textChanged(const QString &text)
{
    QString searchType = ui->comboBoxSearch->currentText(); // Get the selected search type from the combo box
    QSqlQuery query;

    if (text.isEmpty()) {
        // If the search field is empty, reset the table view to show all equipment
        query.prepare("SELECT * FROM equipement");
    } else {
        // Determine which column to search based on the combo box value
        if (searchType == "Nom") {
            query.prepare("SELECT * FROM equipement WHERE NOM_EQ LIKE :term");
        } else if (searchType == "Marque") {
            query.prepare("SELECT * FROM equipement WHERE MARQUE_EQ LIKE :term");
        } else if (searchType == "Reference") {
            query.prepare("SELECT * FROM equipement WHERE REFERENCE_EQ LIKE :term");
        } else {
            QMessageBox::warning(this, "Erreur", "Critère de recherche invalide !");
            return;
        }
        query.bindValue(":term", text + "%"); // Matches rows where the column starts with the input text
    }

    // Execute the query and update the table view
    if (query.exec()) {
        QSqlQueryModel *model = new QSqlQueryModel();
        model->setQuery(query);
        ui->tableViewEquipement->setModel(model); // Dynamically update the tableView
    } else {
        qDebug() << "Search query error:" << query.lastError().text();
    }
}



void MainWindow::createPieChartPost(QFrame *frame, const QString &title, QStringList categories, const QMap<QString, QList<int>> &data)
{
    QPieSeries *series = new QPieSeries();

    int totalSum = 0;

    // Calculate the total sum of all data values
    for (const QString &category : categories) {
        if (data.contains(category)) {
            const QList<int> &categoryData = data[category];
            for (int value : categoryData) {
                totalSum += value;
            }
        }
    }

    // Iterate through equipment categories (e.g., types or states) to create slices with percentages
    for (const QString &category : categories) {
        if (data.contains(category)) {
            const QList<int> &categoryData = data[category];
            int categorySum = 0;

            for (int value : categoryData) {
                categorySum += value;
            }

            // Calculate percentage and create a label
            double percentage = (totalSum > 0) ? (static_cast<double>(categorySum) / totalSum) * 100 : 0.0;
            QString sliceLabel = QString("%1 (%2%)").arg(category).arg(QString::number(percentage, 'f', 1));
            QPieSlice *slice = new QPieSlice(sliceLabel, categorySum);
            series->append(slice);
        }
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(title);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // Clear existing contents of the frame and add the new chart view
    QLayout *layout = frame->layout();
    if (layout) {
        QLayoutItem *item;
        while ((item = layout->takeAt(0))) {
            delete item->widget();
            delete item;
        }
    }
    frame->layout()->addWidget(chartView);
}



void MainWindow::equip_loadChart(QString chartType)
{
    QDate currentDate = QDate::currentDate();
    QString currentDateStr = currentDate.toString("yyyy-MM-dd");

    if (chartType == "Type")
    {
        QMap<QString, QList<int>> typeData = e.retrieveTypeData();
        QStringList categories = {"Machine", "Materiel"};
        createPieChartPost(ui->equipementChartFrame, "Equipment Type Distribution (" + currentDateStr + ")", categories, typeData);
    }
}



void MainWindow::on_tableViewEquipement_clicked(const QModelIndex &index)
{
    QSqlQueryModel* model = qobject_cast<QSqlQueryModel*>(ui->tableViewEquipement->model());


    if (model && index.isValid())
        {
            int row = index.row();
            QMap<QString, QString> rowData; // Create a QMap to store the data
            int numColumns = model->columnCount();

            for (int col = 0; col < numColumns; ++col) {
                QModelIndex itemIndex = model->index(row, col);
                QString columnName = model->headerData(col, Qt::Horizontal).toString().toLower();
                QString itemText = model->data(itemIndex).toString();
                rowData[columnName] = itemText;
            }


            // Fill the line edits with data from the selected row
        ui->lineEditReference->setText(rowData["reference_eq"]);
        ui->lineEditQuantite->setText(rowData["quantite_eq"]);
        ui->lineEditPrix->setText(rowData["prix_eq"]);
        ui->lineEditNom->setText(rowData["nom_eq"]);
        ui->lineEditMarque->setText(rowData["marque_eq"]);
        ui->comboBoxType->setCurrentText(rowData["type_eq"]);
        ui->comboBoxEtat->setCurrentText(rowData["etat_eq"]);

        temp_eq.setreference(rowData["reference_eq"].toInt());
    }
}



////////////////////////////////////mailing///////////////////////////////////////////////////

void MainWindow::on_email_Btn_clicked()
{
    QString recipient = ui->lineEditMail->text();
    QString reference = ui->lineEditReference_2->text();
    bool isModeUtilisationChecked = ui->checkBoxGuide->isChecked();
    bool isFeedbackFormChecked = ui->checkBoxFormulaire->isChecked();

    if (recipient.isEmpty() || reference.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields.");
        return;
    }

    // Check if the reference exists in the database
    QSqlQuery query;
    query.prepare("SELECT REFERENCE_EQ, TYPE_EQ FROM equipement WHERE REFERENCE_EQ = :reference");
    query.bindValue(":reference", reference);

    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, "Error", "The equipment reference does not exist.");
        return;
    }

    QString type = query.value("TYPE_EQ").toString(); // Retrieve the equipment type

    // Prepare subject and body based on the selected checkbox
    QString subject, body;

    if (isModeUtilisationChecked) {
        subject = "Mode d'Utilisation for Equipment " + reference;
        if (type == "Machine") {
            body = "Dear User,\n\n"
                   "Here is the detailed mode d'utilisation for the machine with reference " + reference + ":\n\n"
                   "1. **Inspection**:\n"
                   "   - Ensure the machine is in good condition before use.\n"
                   "   - Check for any visible damage or wear.\n\n"
                   "2. **Setup**:\n"
                   "   - Place the machine on a stable and even surface.\n"
                   "   - Connect to the appropriate power source if required.\n"
                   "   - Ensure all safety measures are in place.\n\n"
                   "3. **Usage**:\n"
                   "   - Follow the operational guidelines provided in the user manual.\n"
                   "   - Avoid exceeding the recommended limits (e.g., weight, pressure, duration).\n"
                   "   - Always operate the machine under supervision if necessary.\n\n"
                   "4. **Maintenance**:\n"
                   "   - Clean the machine after use to ensure longevity.\n"
                   "   - Perform regular maintenance as specified in the manual.\n"
                   "   - Replace any damaged parts immediately to avoid further issues.\n\n"
                   "5. **Storage**:\n"
                   "   - Store the machine in a cool, dry place when not in use.\n"
                   "   - Protect it from dust and moisture.\n\n"
                   "Please adhere to these instructions to ensure the machine operates efficiently and safely.\n\n"
                   "Best regards,\n"
                   "Cleansy Management Team";
        } else if (type == "Materiel") {
            body = "Dear User,\n\n"
                   "Here is the detailed mode d'utilisation for the material with reference " + reference + ":\n\n"
                   "1. **Inspection**:\n"
                   "   - Inspect the material for any defects or irregularities before use.\n\n"
                   "2. **Usage**:\n"
                   "   - Follow the application guidelines provided.\n"
                   "   - Avoid exceeding the recommended usage limits.\n\n"
                   "3. **Storage**:\n"
                   "   - Store the material in a safe, dry place.\n"
                   "   - Ensure proper sealing to maintain quality.\n\n"
                   "Best regards,\n"
                   "Cleansy Management Team";
        }
    } else if (isFeedbackFormChecked) {
        subject = "Feedback Request for Equipment " + reference;
        body = "Dear User,\n\n"
               "We highly value your input and would like to gather your feedback regarding the equipment with reference " + reference + ".\n\n"
               "Please fill out the feedback form at the following link:\n"
               "https://forms.gle/CZkmGWk4bwjWGWVTA\n\n"
               "Thank you for your time and cooperation.\n\n"
               "Best regards,\n"
               "Cleansy Management Team";
    } else {
        QMessageBox::warning(this, "Error", "Please select a checkbox.");
        return;
    }

    // Send email
    Mailing mailer;
    mailer.sendEmail(recipient, subject, body);
    QMessageBox::information(this, "Email", "Email sent successfully!");
}

///////////////////// qr code ////////////////////////


void MainWindow::on_generateQRCodeButton_clicked()
{
    // Get the equipment reference from the line edit
    QString reference = ui->lineEditReference3->text();

    // Check if the reference is empty
    if (reference.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Please enter the equipment reference."));
        return;
    }

    // Prepare the SQL query to fetch equipment details based on the reference
    QSqlQuery query;
    query.prepare("SELECT * FROM equipement WHERE REFERENCE_EQ = :reference");
    query.bindValue(":reference", reference);

    // Execute the query
    if (!query.exec()) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to execute query."));
        qDebug() << "SQL Query Error:" << query.lastError().text();
        return;
    }

    // Check if a record is found
    QString data;
    if (query.next()) {
        // Add a clear title to make it easy to understand
        data = "Equipment Info\n";

        // Collect data from the query result and format it for the QR code
        for (int i = 0; i < query.record().count(); ++i) {
            QString fieldName = query.record().fieldName(i);
            QString fieldValue = query.value(i).toString();
            if (!fieldValue.isEmpty()) {
                data += fieldName + ": " + fieldValue + "\n";
            }
        }
    } else {
        QMessageBox::warning(this, tr("Error"), tr("No data found for the specified reference."));
        return;
    }

    // Debug: Check the collected data
    qDebug() << "Data for QR Code:\n" << data;

    // Generate the QR code from the formatted data
    QrCode qr = QrCode::encodeText(data.toUtf8().constData(), qrcodegen::QrCode::Ecc::HIGH);

    // Debug: Check QR code size
    qDebug() << "QR Code Size: " << qr.getSize();

    // Convert the QR code into a QImage for displaying
    QImage qrImage(qr.getSize(), qr.getSize(), QImage::Format_RGB32);
    for (int y = 0; y < qr.getSize(); ++y) {
        for (int x = 0; x < qr.getSize(); ++x) {
            bool color = qr.getModule(x, y);
            qrImage.setPixelColor(x, y, color ? QColor("#000000") : QColor("#FFFFFF"));
        }
    }

    // Resize and set the QR code to the QLabel
    QPixmap resizedQrPixmap = QPixmap::fromImage(qrImage).scaled(140,200, Qt::KeepAspectRatio);

    // Debug: Check the resized QR code size
    qDebug() << "Resized QR Code Size: " << resizedQrPixmap.size();

    // Set the QR code to the QLabel
    ui->qr_code->setPixmap(resizedQrPixmap);  // Assuming qr_code is the QLabel for the QR code
}





