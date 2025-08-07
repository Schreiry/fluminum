#pragma once
#include "Common.h"

// --- Console Formatting ---
void print_header_box(const string& title, int width = 80);
void print_footer_box(int width = 80);
void print_line_in_box(const std::string& content, int width = 80, bool add_color_reset_at_end = true, Alignment alignment = Alignment::Left);
void print_matrix_preview(const Matrix& m, std::ostream& os = std::cout, int precision = 3, int max_print_dim = 10);
void display_intro_banner();

// --- User Input ---
template<typename T>
T get_valid_input(const std::string& prompt_text);
void clear_input_buffer_after_cin();

// --- File I/O ---
Matrix readMatrixFromFile(const std::string& filename);
void saveMatrixToFile(const Matrix& matrix, const std::string& filename);

// --- Logging ---
void logMultiplicationResultToCSV(const MultiplicationResult& result, const std::string& filename);
void logComparisonResultToCSV(const ComparisonResult& result, const std::string& filename);

// --- UI Feedback ---
void play_completion_sound();
const string SPINNER_CHARS[] = { CYAN + "|" + RESET, YELLOW + "/" + RESET, BLUE + "-" + RESET, PURPLE + "\\" + RESET };
const int NUM_SPINNER_CHARS = 4;
void show_loading_animation_step(int& spinner_idx, const std::string& message);