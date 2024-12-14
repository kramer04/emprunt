#include <gtkmm.h>
#include <gdkmm/pixbuf.h>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>

//-------------------------------------------------------------
// 1) Fonctions financières
//-------------------------------------------------------------
double calculate_capital(double repayment, double nb_periods, double annual_rate)
{
  double monthly_rate = annual_rate / 12.0;
  return repayment * (1 - std::pow(1 + monthly_rate, -nb_periods)) / monthly_rate;
}

double calculate_repayment(double capital, double nb_periods, double annual_rate)
{
  double monthly_rate = annual_rate / 12.0;
  return capital * monthly_rate / (1 - std::pow(1 + monthly_rate, -nb_periods));
}

double calculate_duration(double capital, double repayment, double annual_rate)
{
  double monthly_rate = annual_rate / 12.0;
  return -std::log(1 - (capital * monthly_rate / repayment)) / std::log(1 + monthly_rate);
}

double find_interest_rate(double capital, double repayment, double nb_periods,
  double tol = 1e-6, int max_iter = 100)
{
  double annual_rate = 0.05;
  for (int i = 0; i < max_iter; i++)
  {
    double monthly_rate = annual_rate / 12.0;
    double f_val = repayment - capital * monthly_rate / (1 - std::pow(1 + monthly_rate, -nb_periods));

    double df_dmr = -capital * (1 - std::pow(1 + monthly_rate, -nb_periods))
      + capital * monthly_rate * nb_periods * std::pow(1 + monthly_rate, -nb_periods);
    df_dmr /= std::pow((1 - std::pow(1 + monthly_rate, -nb_periods)), 2);
    double f_prime_val = df_dmr / 12.0;

    double annual_rate_new = annual_rate - f_val / f_prime_val;
    if (std::fabs(annual_rate_new - annual_rate) < tol)
      return annual_rate_new;
    annual_rate = annual_rate_new;
  }
  std::cerr << "Newton-Raphson: pas de convergence.\n";
  return annual_rate;
}

//-------------------------------------------------------------
// 2) Formatage d'un double en "xx xxx,xx"
//-------------------------------------------------------------
static std::string format_number(double value)
{
  // Étape 1 : convertir en "1234.56" (locale C, 2 décimales)
  std::ostringstream oss;
  oss.imbue(std::locale::classic());
  oss << std::fixed << std::setprecision(2) << value; // ex: "1234.56"
  std::string tmp = oss.str(); // "1234.56"

  // Étape 2 : chercher le point '.'
  size_t dot_pos = tmp.find('.');
  if (dot_pos == std::string::npos)
  {
    // pas de point => on force ",00"
    tmp += ",00";
  }
  else
  {
    // remplacer le '.' par ','
    tmp[dot_pos] = ',';
  }

  // Maintenant tmp ressemble à "1234,56"
  // Étape 3 : séparer l’intégrale de la décimale
  dot_pos = tmp.find(',');
  std::string integral = tmp.substr(0, dot_pos);   // "1234"
  std::string decimals = tmp.substr(dot_pos + 1);  // "56"

  // Étape 4 : insérer un espace ' ' ASCII toutes les 3 positions depuis la fin
  int count = 0;
  for (int i = integral.size() - 1; i > 0; i--)
  {
    count++;
    if (count % 3 == 0)
    {
      integral.insert(i, " ");
    }
  }

  // Recomposer "1 234,56"
  return integral + "," + decimals;
}

// Petit helper pour justifier à droite
static std::string pad_right(const std::string &s, int width)
{
  std::ostringstream oss;
  oss << std::right << std::setw(width) << s;
  return oss.str();
}

//-------------------------------------------------------------
// 3) Classe principale : CalculatorWindow
//-------------------------------------------------------------
class CalculatorWindow : public Gtk::ApplicationWindow
{
public:
  CalculatorWindow()
  {
    set_title("Calculateur Financier");
    set_default_size(600, 600);    

    main_box.set_orientation(Gtk::Orientation::VERTICAL);
    main_box.set_margin(10);
    main_box.set_spacing(10);
    set_child(main_box);

    // Combo pour choisir la variable à calculer
    combo_calcul.append("Capital");
    combo_calcul.append("Durée (périodes)");
    combo_calcul.append("Remboursement périodique");
    combo_calcul.append("Taux");
    combo_calcul.set_active_text("Capital");
    combo_calcul.signal_changed().connect(sigc::mem_fun(*this, &CalculatorWindow::on_combo_changed));

    Gtk::Box *top_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
    top_box->set_spacing(10);
    combo_label.set_text("Calcul à effectuer : ");
    top_box->append(combo_label);
    top_box->append(combo_calcul);
    main_box.append(*top_box);

    // Grille : Capital, Remboursement, Durée, Taux
    grid.set_row_spacing(10);
    grid.set_column_spacing(10);

    lbl_capital.set_text("Capital : ");
    lbl_repayment.set_text("Remboursement : ");
    lbl_duration.set_text("Durée (périodes) : ");
    lbl_rate.set_text("Taux annuel (%) : "); // ex: 7.42

    grid.attach(lbl_capital, 0, 0, 1, 1);
    grid.attach(entry_capital, 1, 0, 1, 1);

    grid.attach(lbl_repayment, 0, 1, 1, 1);
    grid.attach(entry_repayment, 1, 1, 1, 1);

    grid.attach(lbl_duration, 0, 2, 1, 1);
    grid.attach(entry_duration, 1, 2, 1, 1);

    grid.attach(lbl_rate, 0, 3, 1, 1);
    grid.attach(entry_rate, 1, 3, 1, 1);

    main_box.append(grid);

    // Barre de boutons
    bottom_box.set_spacing(10);

    btn_result.set_label("Résultat");
    btn_result.signal_clicked().connect(sigc::mem_fun(*this, &CalculatorWindow::on_result_clicked));

    btn_clear.set_label("Effacer");
    btn_clear.signal_clicked().connect(sigc::mem_fun(*this, &CalculatorWindow::on_clear_clicked));

    btn_schedule.set_label("Tableau d'amortissement");
    btn_schedule.signal_clicked().connect(sigc::mem_fun(*this, &CalculatorWindow::on_schedule_clicked));

    btn_quit.set_label("Quitter");
    btn_quit.signal_clicked().connect(sigc::mem_fun(*this, &CalculatorWindow::on_quit_clicked));

    bottom_box.append(btn_result);
    bottom_box.append(btn_clear);
    bottom_box.append(btn_schedule);
    bottom_box.append(btn_quit);

    main_box.append(bottom_box);

    // Zone de défilement pour le tableau
    scrolled_window.set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
    scrolled_window.set_hexpand(true);
    scrolled_window.set_vexpand(true);
    scrolled_window.set_visible(false);

    textview_schedule.set_editable(false);
    textview_schedule.set_monospace(true);  // rendre le TextView monospace
    scrolled_window.set_child(textview_schedule);

    main_box.append(scrolled_window);

    on_combo_changed(); // initial
  }

protected:
  Gtk::Box main_box{Gtk::Orientation::VERTICAL};
  Gtk::ComboBoxText combo_calcul;
  Gtk::Label combo_label{"Calcul à effectuer : "};

  Gtk::Grid grid;
  Gtk::Label lbl_capital, lbl_repayment, lbl_duration, lbl_rate;
  Gtk::Entry entry_capital, entry_repayment, entry_duration, entry_rate;

  Gtk::Box bottom_box{Gtk::Orientation::HORIZONTAL};
  Gtk::Button btn_result, btn_clear, btn_schedule, btn_quit;

  Gtk::ScrolledWindow scrolled_window;
  Gtk::TextView textview_schedule;

  void on_combo_changed()
  {
    Glib::ustring choice = combo_calcul.get_active_text();
    // Déverrouiller tout
    entry_capital.set_editable(true);
    entry_repayment.set_editable(true);
    entry_duration.set_editable(true);
    entry_rate.set_editable(true);

    // Verrouiller la case à calculer
    if (choice == "Capital")
    {
      entry_capital.set_editable(false);
    }
    else if (choice == "Durée (périodes)")
    {
      entry_duration.set_editable(false);
    }
    else if (choice == "Remboursement périodique")
    {
      entry_repayment.set_editable(false);
    }
    else if (choice == "Taux")
    {
      entry_rate.set_editable(false);
    }
  }

  // Bouton Résultat => Calculer variable manquante
  void on_result_clicked()
  {
    Glib::ustring choice = combo_calcul.get_active_text();

    double capital = std::stod(entry_capital.get_text().empty() ? "0.0" : entry_capital.get_text());
    double repay = std::stod(entry_repayment.get_text().empty() ? "0.0" : entry_repayment.get_text());
    double dur = std::stod(entry_duration.get_text().empty() ? "0.0" : entry_duration.get_text());
    double rate_input = std::stod(entry_rate.get_text().empty() ? "0.0" : entry_rate.get_text());
    double annual_rate = rate_input / 100.0; // 7.42 => 0.0742

    if (choice == "Capital")
    {
      double result = calculate_capital(repay, dur, annual_rate);
      entry_capital.set_text(std::to_string(result));
    }
    else if (choice == "Durée (périodes)")
    {
      double result = calculate_duration(capital, repay, annual_rate);
      entry_duration.set_text(std::to_string(result));
    }
    else if (choice == "Remboursement périodique")
    {
      double result = calculate_repayment(capital, dur, annual_rate);
      entry_repayment.set_text(std::to_string(result));
    }
    else if (choice == "Taux")
    {
      double found_rate = find_interest_rate(capital, repay, dur);
      double percent_rate = found_rate * 100.0;
      entry_rate.set_text(std::to_string(percent_rate));
    }
  }

  // Bouton Effacer
  void on_clear_clicked()
  {
    entry_capital.set_text("");
    entry_repayment.set_text("");
    entry_duration.set_text("");
    entry_rate.set_text("");
    textview_schedule.get_buffer()->set_text("");
    scrolled_window.set_visible(false);
  }

  // Bouton Tableau d'amortissement
  void on_schedule_clicked()
  {
    double capital = std::stod(entry_capital.get_text().empty() ? "0.0" : entry_capital.get_text());
    double repay = std::stod(entry_repayment.get_text().empty() ? "0.0" : entry_repayment.get_text());
    double dur = std::stod(entry_duration.get_text().empty() ? "0.0" : entry_duration.get_text());
    double rate_input = std::stod(entry_rate.get_text().empty() ? "0.0" : entry_rate.get_text());
    double annual_rate = rate_input / 100.0;

    if (capital <= 0 || repay <= 0 || dur <= 0 || annual_rate <= 0)
    {
      textview_schedule.get_buffer()->set_text("Paramètres invalides pour générer le tableau d'amortissement.");
      scrolled_window.set_visible(true);
      return;
    }

    std::stringstream ss;
    ss << pad_right("Période", 9) << " | "
      << pad_right("Intérêt", 17) << " | "
      << pad_right("Principal", 15) << " | "
      << pad_right("Reste", 15) << "\n";
    ss << "--------------------------------------------------------------\n";

    double monthly_rate = annual_rate / 12.0;
    double remaining = capital;

    for (int period = 1; period <= static_cast<int>(dur); period++)
    {
      double interest = remaining * monthly_rate;
      double principal = repay - interest;
      if (principal < 0) principal = 0.0;
      remaining -= principal;
      if (remaining < 0) remaining = 0.0;

      // Affichage "xx xxx,xx"
      std::string interest_str = format_number(interest);
      std::string principal_str = format_number(principal);
      std::string remain_str = format_number(remaining);

      ss << pad_right(std::to_string(period), 8) << " | "
        << pad_right(interest_str, 15) << " | "
        << pad_right(principal_str, 15) << " | "
        << pad_right(remain_str, 15) << "\n";

      if (remaining <= 0) break;
    }

    textview_schedule.get_buffer()->set_text(ss.str());
    scrolled_window.set_visible(true);
  }

  void on_quit_clicked()
  {
    close();
  }
};

//-------------------------------------------------------------
// 4) main
//-------------------------------------------------------------
int main(int argc, char *argv[])
{
  auto app = Gtk::Application::create("org.example.calculatrice_financiere");
  return app->make_window_and_run<CalculatorWindow>(argc, argv);
}
