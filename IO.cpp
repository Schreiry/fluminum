#define NOMINMAX
#include "IO.h"
#include "Matrix.h" // For Matrix object interactions

// --- Console Formatting ---

int get_visible_width(const std::string& text) {
    int visible_width = 0;
    bool in_escape_sequence = false;
    for (char c : text) {
        if (in_escape_sequence) {
            if (c == 'm') in_escape_sequence = false;
        }
        else if (c == '\033') {
            in_escape_sequence = true;
        }
        else {
            visible_width++;
        }
    }
    return visible_width;
}

void print_hline(int width) {
    for (int i = 0; i < width; ++i) cout << BOX_HLINE;
}

void print_separator_line(int width = 80) {
    cout << BOX_LTEE; print_hline(width - 2); cout << BOX_RTEE << endl;
}

void print_header_box(const string& title, int width) {
    int title_visible_len = get_visible_width(title);
    int padding_total = width - title_visible_len - 4;
    int padding_left = padding_total / 2;
    int padding_right = padding_total - padding_left;

    cout << BOX_TLCORNER; print_hline(width - 2); cout << BOX_TRCORNER << endl;
    cout << BOX_VLINE << std::string(padding_left > 0 ? padding_left : 0, ' ') << " " << title << " " << std::string(padding_right > 0 ? padding_right : 0, ' ') << RESET << BOX_VLINE << endl;
    cout << BOX_LTEE; print_hline(width - 2); cout << BOX_RTEE << endl;
}

void print_footer_box(int width) {
    cout << BOX_BLCORNER; print_hline(width - 2); cout << BOX_BRCORNER << endl;
}

void print_line_in_box(const std::string& content, int width, bool add_color_reset_at_end, Alignment alignment) {
    int box_content_width = width - 2;
    int content_visible_width = get_visible_width(content);
    int padding = box_content_width - content_visible_width;
    if (padding < 0) padding = 0;

    cout << BOX_VLINE;
    string processed_content = content;

    if (alignment == Alignment::Left) {
        cout << processed_content << std::string(padding, ' ');
    }
    else if (alignment == Alignment::Right) {
        cout << std::string(padding, ' ') << processed_content;
    }
    else if (alignment == Alignment::Center) {
        cout << std::string(padding / 2, ' ') << processed_content << std::string(padding - (padding / 2), ' ');
    }
    if (add_color_reset_at_end && !(processed_content.length() >= RESET.length() && processed_content.substr(processed_content.length() - RESET.length()) == RESET)) {
        cout << RESET;
    }
    cout << BOX_VLINE << endl;
}

void print_matrix_preview(const Matrix& m, std::ostream& os, int precision, int max_print_dim) {
    std::ios_base::fmtflags original_flags = os.flags();
    std::streamsize original_precision = os.precision();
    os << std::fixed << std::setprecision(precision);

    os << YELLOW << "Matrix (" << m.rows() << "x" << m.cols() << "):" << RESET << "\n";
    if (m.isEmpty()) {
        os << DARK_GRAY << "(Empty Matrix)" << RESET << "\n" << endl;
        os.flags(original_flags); os.precision(original_precision);
        return;
    }

    int print_rows = std::min(m.rows(), max_print_dim);
    int print_cols = std::min(m.cols(), max_print_dim);

    for (int i = 0; i < print_rows; ++i) {
        os << "[ ";
        for (int j = 0; j < print_cols; ++j) {
            os << std::setw(precision + 5) << m(i, j) << (j == print_cols - 1 ? "" : " ");
        }
        if (m.cols() > print_cols) os << " ... ";
        os << " ]\n";
    }
    if (m.rows() > print_rows) os << "  ...\n";
    os << std::endl;

    os.flags(original_flags);
    os.precision(original_precision);
}

void display_intro_banner() {
    cout << R"(                                                                                                                   
                                   ?                  
                         ??    ??                      
                     ?     ?                           
                    ?       ??                         
                      ???        ???                   
                          ???         ??               
                             ??         ??             
                              ?         ??             
                           ??           ??             
                       ???           ? ??              
                  ???              ? ??                
              ??                ????                   
            ?               ? ???                      
           ?               ? ?                         
                           ?                           
                                                       
                                                       
   ???? ?    ?   ?  ?    ?  ? ??   ? ?   ?  ?    ?     
   ?    ?    ?   ?  ??  ??  ? ???  ? ?   ?  ??  ??     
   ?    ?    ??  ?  ? ?? ?  ? ?? ??? ?   ?  ? ?? ?     
   ?    ????   ??   ?    ?  ?  ?   ?   ??   ?    ?                                                                                                                                                                                                                                                                                                                                                                                         
  )" << endl;
}

// --- User Input ---
void clear_input_buffer_after_cin() {
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

template<typename T>
T get_valid_input(const std::string& prompt_text) {
    T value;
    while (true) {
        cout << prompt_text << YELLOW;
        cin >> value;
        cout << RESET;
        if (cin.good()) {
            if (!std::is_same<T, std::string>::value) {
                char next_char = static_cast<char>(cin.peek());
                if (next_char != '\n' && next_char != EOF && !isspace(next_char)) {
                    cerr << RED << "\nInvalid input: Extra characters after the number. Please try again." << RESET << endl;
                    cin.clear();
                    clear_input_buffer_after_cin();
                    continue;
                }
            }
            clear_input_buffer_after_cin();
            return value;
        }
        else {
            cerr << RED << "\nInvalid input format. Please try again." << RESET << endl;
            cin.clear();
            clear_input_buffer_after_cin();
        }
    }
}
// Explicit template instantiations
template int get_valid_input<int>(const std::string&);
template unsigned int get_valid_input<unsigned int>(const std::string&);
template double get_valid_input<double>(const std::string&);
template char get_valid_input<char>(const std::string&);


template<>
string get_valid_input<string>(const std::string& prompt_text) {
    string value;
    while (true) {
        cout << prompt_text << YELLOW;
        std::getline(cin >> std::ws, value);
        cout << RESET;
        if (!value.empty()) {
            return value;
        }
        else {
            cerr << RED << "\nInput cannot be empty. Please try again." << RESET << endl;
        }
    }
}


// --- File I/O ---
Matrix readMatrixFromFile(const std::string& filename) {
    std::ifstream infile(filename);
    if (!infile.is_open()) throw std::runtime_error("Could not open file: " + filename);

    // Skip UTF-8 BOM if present
    char bom[3] = { 0 };
    infile.read(bom, 3);
    if (static_cast<unsigned char>(bom[0]) != 0xEF || static_cast<unsigned char>(bom[1]) != 0xBB || static_cast<unsigned char>(bom[2]) != 0xBF) {
        infile.seekg(0); // Not a BOM, rewind
    }

    std::vector<std::vector<double>> temp_data;
    string line;
    int expected_cols = -1;
    int line_num = 0;
    int spinner_idx = 0;
    const char separator = ',';

    cout << CYAN << "Reading formatted matrix from file: " << filename << RESET;
    auto last_update_time = std::chrono::steady_clock::now();

    while (std::getline(infile, line)) {
        line_num++;
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        if (line.empty() || line[0] == separator) continue;

        std::vector<double> row_vec;
        std::stringstream ss(line);
        string segment;
        std::vector<string> segments;
        while (std::getline(ss, segment, separator)) segments.push_back(segment);

        if (segments.size() < 3) continue; // Skip non-data rows

        for (size_t i = 1; i < segments.size() - 1; ++i) {
            try {
                row_vec.push_back(std::stod(segments[i]));
            }
            catch (const std::exception&) {
                cout << "\r" << string(80, ' ') << "\r"; infile.close();
                throw std::invalid_argument("Malformed number '" + segments[i] + "' in " + filename + " at line " + std::to_string(line_num));
            }
        }
        if (expected_cols == -1) {
            expected_cols = static_cast<int>(row_vec.size());
        }
        else if (static_cast<int>(row_vec.size()) != expected_cols) {
            cout << "\r" << string(80, ' ') << "\r"; infile.close();
            throw std::invalid_argument("Inconsistent columns in " + filename + " at line " + std::to_string(line_num));
        }

        temp_data.push_back(row_vec);

        auto current_time = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_update_time).count() > 100) {
            show_loading_animation_step(spinner_idx, "Reading data rows: " + std::to_string(temp_data.size()));
            last_update_time = current_time;
        }
    }
    cout << "\r" << string(80, ' ') << "\r";
    infile.close();

    if (temp_data.empty()) {
        cout << YELLOW << "Warning: File '" << filename << "' contained no valid data rows. Creating 0x0 matrix." << RESET << endl;
        return Matrix(0, 0);
    }
    cout << GREEN << "Successfully read " << temp_data.size() << " data rows from file." << RESET << endl;
    return Matrix(temp_data);
}

void saveMatrixToFile(const Matrix& matrix, const std::string& filename) {
    std::ofstream outfile(filename, std::ios::binary);
    if (!outfile.is_open()) throw std::runtime_error("Could not open file for writing: " + filename);

    outfile << (char)0xEF << (char)0xBB << (char)0xBF; // UTF-8 BOM

    print_header_box("Saving to " + filename, 80);

    if (matrix.isEmpty()) {
        print_line_in_box(YELLOW + "Matrix is empty. Saving header-only CSV file." + RESET, 80);
        outfile << "Y-Axis,X-Axis" << endl;
        outfile.close();
        print_footer_box(80);
        cout << GREEN << "Empty matrix info saved to " << filename << RESET << endl << endl;
        return;
    }

    const char separator = ',';
    const std::string arrow_r = "\u25B6", arrow_l = "\u25C0", arrow_d = "\u25BC", arrow_u = "\u25B2";
    const int precision = 8;
    outfile << std::scientific << std::setprecision(precision);

    // Top Header
    outfile << " " << separator;
    for (int j = 0; j < matrix.cols(); ++j) outfile << "\"" << arrow_d << " " << format_coord(j) << " " << arrow_d << "\"" << separator;
    outfile << " " << endl;

    // Data
    for (int i = 0; i < matrix.rows(); ++i) {
        outfile << "\"" << arrow_r << " " << format_coord(i) << " " << arrow_r << "\"" << separator;
        for (int j = 0; j < matrix.cols(); ++j) outfile << matrix(i, j) << separator;
        outfile << "\"" << arrow_l << " " << format_coord(i) << " " << arrow_l << "\"" << endl;
    }

    // Bottom Header
    outfile << " " << separator;
    for (int j = 0; j < matrix.cols(); ++j) outfile << "\"" << arrow_u << " " << format_coord(j) << " " << arrow_u << "\"" << separator;
    outfile << " " << endl;

    outfile.close();

    print_footer_box(80);
    if (outfile.fail()) cerr << RED << "Error writing or closing file: " << filename << RESET << endl;
    else cout << GREEN << "Matrix successfully saved to " << filename << RESET << endl << endl;
}

// --- Logging ---
void logMultiplicationResultToCSV(const MultiplicationResult& result, const std::string& filename) {
    std::ofstream logfile(filename, std::ios::out | std::ios::app);
    if (!logfile.is_open()) {
        cerr << RED << "Error: Could not open log file: " << filename << RESET << endl; return;
    }

    if (logfile.tellp() == 0) {
        logfile << "Operation,RowsA,ColsA,RowsB,ColsB,ResultRows,ResultCols,TotalElementsResult,"
            << "DurationSeconds_Chrono,DurationNanoseconds_Chrono,DurationSeconds_QPC,"
            << "ThreadsUsed,CoresDetected,PeakMemoryMB,StrassenThreshold,"
            << "StrassenAppliedTopLevel,Padding_sec,Unpadding_sec,"
            << "Split_L1_sec,S_Calc_L1_sec,P_Tasks_L1_Wall_sec,C_Quad_Calc_L1_sec,Final_Combine_L1_sec\n";
    }

    logfile << std::fixed << std::setprecision(10);
    logfile << "Multiplication,"
        << result.originalRowsA << "," << result.originalColsA << ","
        << result.originalRowsB << "," << result.originalColsB << ","
        << result.resultMatrix.rows() << "," << result.resultMatrix.cols() << ","
        << result.resultMatrix.elementCount() << ","
        << result.durationSeconds_chrono << "," << result.durationNanoseconds_chrono << ","
        << result.durationSeconds_qpc << "," << result.threadsUsed << ","
        << result.coresDetected << "," << result.memoryInfo.peakWorkingSetMB << ","
        << result.strassenThreshold << ","
        << (result.strassen_applied_at_top_level ? "Yes" : "No") << ","
        << result.padding_duration_sec << "," << result.unpadding_duration_sec << ",";

    if (result.strassen_applied_at_top_level) {
        logfile << result.first_level_split_sec << "," << result.first_level_S_calc_sec << ","
            << result.first_level_P_tasks_wall_sec << "," << result.first_level_C_quad_calc_sec << ","
            << result.first_level_final_combine_sec;
    }
    else {
        logfile << "0.0,0.0,0.0,0.0,0.0";
    }
    logfile << "\n";
    logfile.close();
    cout << GREEN << "Multiplication result logged to " << filename << RESET << endl;
}

void logComparisonResultToCSV(const ComparisonResult& result, const std::string& filename) {
    std::ofstream logfile(filename, std::ios::out | std::ios::app);
    if (!logfile.is_open()) {
        cerr << RED << "Error: Could not open log file: " << filename << RESET << endl; return;
    }

    if (logfile.tellp() == 0) {
        logfile << "Operation,Rows,Cols,TotalElements,MatchCount,MismatchCount,MatchPercentage,"
            << "DurationSeconds_Chrono,DurationNanoseconds_Chrono,DurationSeconds_QPC,"
            << "ThreadsUsed,CoresDetected,PeakMemoryMB,ComparisonThreshold,Epsilon\n";
    }

    long long total_elements = static_cast<long long>(result.originalRows) * result.originalCols;
    double match_percentage = (total_elements > 0) ? (static_cast<double>(result.matchCount) / total_elements) * 100.0 : 0.0;

    logfile << "Comparison,"
        << result.originalRows << "," << result.originalCols << "," << total_elements << ","
        << result.matchCount << "," << (total_elements - result.matchCount) << ","
        << std::fixed << std::setprecision(2) << match_percentage << ","
        << std::fixed << std::setprecision(10) << result.durationSeconds_chrono << ","
        << result.durationNanoseconds_chrono << "," << result.durationSeconds_qpc << ","
        << result.threadsUsed << "," << result.coresDetected << ","
        << result.memoryInfo.peakWorkingSetMB << "," << result.comparisonThreshold << ","
        << std::scientific << std::setprecision(10) << result.epsilon << "\n";

    logfile.close();
    cout << GREEN << "Comparison result logged to " << filename << RESET << endl;
}

// --- UI Feedback ---
void show_loading_animation_step(int& spinner_idx, const std::string& message) {
    string display_message = message;
    if (get_visible_width(message) > 60) {
        display_message = message.substr(0, 57) + "...";
    }
    cout << "\r" << display_message << " " << SPINNER_CHARS[spinner_idx] << " " << std::flush;
    spinner_idx = (spinner_idx + 1) % NUM_SPINNER_CHARS;
}

void play_completion_sound() {
#ifdef _WIN32
    Beep(623, 150); // D#5
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    Beep(783, 200); // G5
#else
    cout << '\a' << std::flush; // Standard terminal bell
#endif
}