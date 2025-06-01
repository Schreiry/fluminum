import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as mticker
import numpy as np # For calculating bar positions

# Processor performance data
# This list of dictionaries contains test results for various processors
# at different thread counts.
# 'Processor': Name of the processor
# 'Threads': Number of threads used
# 'OM_Time': Execution time for OM test (in seconds)
# 'SA_Time': Execution time for SA test (in seconds)
data = [
    #Intel Core i9-14900
    {'Processor': 'Intel Core i9-14900', 'Threads': 1, 'OM_Time': 212.125, 'SA_Time': 1.766},
    {'Processor': 'Intel Core i9-14900', 'Threads': 2, 'OM_Time': 202.713, 'SA_Time': 1.738506},
    {'Processor': 'Intel Core i9-14900', 'Threads': 3, 'OM_Time': 205.084, 'SA_Time': 1.591649},
    {'Processor': 'Intel Core i9-14900', 'Threads': 4, 'OM_Time': 213.132, 'SA_Time': 1.59984},
    {'Processor': 'Intel Core i9-14900', 'Threads': 5, 'OM_Time': 207.081, 'SA_Time': 1.588758},
    {'Processor': 'Intel Core i9-14900', 'Threads': 6, 'OM_Time': 211.841, 'SA_Time': 1.597039},
    {'Processor': 'Intel Core i9-14900', 'Threads': 7, 'OM_Time': 205.174, 'SA_Time': 1.295759},
    {'Processor': 'Intel Core i9-14900', 'Threads': 8, 'OM_Time': 206.693, 'SA_Time': 1.328877},
    {'Processor': 'Intel Core i9-14900', 'Threads': 9, 'OM_Time': 214.574, 'SA_Time': 1.351797},
    {'Processor': 'Intel Core i9-14900', 'Threads': 10, 'OM_Time': 213.191, 'SA_Time': 1.200275},
    {'Processor': 'Intel Core i9-14900', 'Threads': 11, 'OM_Time': 205.221, 'SA_Time': 1.209206},
    {'Processor': 'Intel Core i9-14900', 'Threads': 12, 'OM_Time': 213.158, 'SA_Time': 1.205571},
    {'Processor': 'Intel Core i9-14900', 'Threads': 13, 'OM_Time': 202.988, 'SA_Time': 1.334024},
    {'Processor': 'Intel Core i9-14900', 'Threads': 14, 'OM_Time': 211.949, 'SA_Time': 1.189405},
    {'Processor': 'Intel Core i9-14900', 'Threads': 15, 'OM_Time': 217.023, 'SA_Time': 1.19356},
    {'Processor': 'Intel Core i9-14900', 'Threads': 16, 'OM_Time': 207.457, 'SA_Time': 1.202011},
    {'Processor': 'Intel Core i9-14900', 'Threads': 17, 'OM_Time': 217.485, 'SA_Time': 1.312161},
    {'Processor': 'Intel Core i9-14900', 'Threads': 18, 'OM_Time': 215.522, 'SA_Time': 1.187201},
    {'Processor': 'Intel Core i9-14900', 'Threads': 19, 'OM_Time': 204.714, 'SA_Time': 1.169427},
    {'Processor': 'Intel Core i9-14900', 'Threads': 20, 'OM_Time': 206.158, 'SA_Time': 1.340084},
    {'Processor': 'Intel Core i9-14900', 'Threads': 21, 'OM_Time': 206.063, 'SA_Time': 1.217577},
    {'Processor': 'Intel Core i9-14900', 'Threads': 22, 'OM_Time': 201.047, 'SA_Time': 1.183273},
    {'Processor': 'Intel Core i9-14900', 'Threads': 23, 'OM_Time': 217.463, 'SA_Time': 1.309612},
    {'Processor': 'Intel Core i9-14900', 'Threads': 24, 'OM_Time': 214.093, 'SA_Time': 1.177822},
    {'Processor': 'Intel Core i9-14900', 'Threads': 25, 'OM_Time': 210.082, 'SA_Time': 1.183906},
    {'Processor': 'Intel Core i9-14900', 'Threads': 26, 'OM_Time': 217.099, 'SA_Time': 1.32088},
    {'Processor': 'Intel Core i9-14900', 'Threads': 27, 'OM_Time': 216.215, 'SA_Time': 1.169328},
    {'Processor': 'Intel Core i9-14900', 'Threads': 28, 'OM_Time': 207.935, 'SA_Time': 1.316717},
    {'Processor': 'Intel Core i9-14900', 'Threads': 29, 'OM_Time': 211.484, 'SA_Time': 1.117219},
    {'Processor': 'Intel Core i9-14900', 'Threads': 30, 'OM_Time': 217.729, 'SA_Time': 1.322540},
    {'Processor': 'Intel Core i9-14900', 'Threads': 31, 'OM_Time': 214.930, 'SA_Time': 1.005802},
    {'Processor': 'Intel Core i9-14900', 'Threads': 32, 'OM_Time': 212.960, 'SA_Time': 0.889268},

    # Data for Intel Core i9-13900K
    {'Processor': 'Intel Core i9-13900K', 'Threads': 1, 'OM_Time': 165.998, 'SA_Time': 1.7941},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 2, 'OM_Time': 172.821, 'SA_Time': 1.7591},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 3, 'OM_Time': 173.704, 'SA_Time': 1.7450},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 4, 'OM_Time': 173.420, 'SA_Time': 1.3633},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 5, 'OM_Time': 167.302, 'SA_Time': 1.3606},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 6, 'OM_Time': 179.375, 'SA_Time': 1.2974},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 7, 'OM_Time': 177.839, 'SA_Time': 1.2219},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 8, 'OM_Time': 170.422, 'SA_Time': 0.9992},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 9, 'OM_Time': 171.258, 'SA_Time': 0.9981},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 10, 'OM_Time': 173.748, 'SA_Time': 0.9709},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 11, 'OM_Time': 181.269, 'SA_Time': 0.9746},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 12, 'OM_Time': 179.265, 'SA_Time': 0.9603},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 13, 'OM_Time': 178.592, 'SA_Time': 0.9578},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 14, 'OM_Time': 173.257, 'SA_Time': 0.9928},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 15, 'OM_Time': 174.146, 'SA_Time': 0.9478},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 16, 'OM_Time': 167.549, 'SA_Time': 0.9428},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 17, 'OM_Time': 167.967, 'SA_Time': 0.8903},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 18, 'OM_Time': 161.175, 'SA_Time': 0.8778},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 19, 'OM_Time': 176.955, 'SA_Time': 0.8553},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 20, 'OM_Time': 159.841, 'SA_Time': 0.8328},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 21, 'OM_Time': 158.875, 'SA_Time': 0.6303},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 22, 'OM_Time': 180.061, 'SA_Time': 0.6102},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 23, 'OM_Time': 173.793, 'SA_Time': 0.5891},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 24, 'OM_Time': 179.338, 'SA_Time': 0.5230},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 25, 'OM_Time': 180.219, 'SA_Time': 0.5115},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 26, 'OM_Time': 181.192, 'SA_Time': 0.5000},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 27, 'OM_Time': 177.954, 'SA_Time': 0.4885},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 28, 'OM_Time': 166.130, 'SA_Time': 0.4170},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 29, 'OM_Time': 176.288, 'SA_Time': 0.4055},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 30, 'OM_Time': 178.632, 'SA_Time': 0.3540},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 31, 'OM_Time': 171.817, 'SA_Time': 0.3425},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 32, 'OM_Time': 171.786, 'SA_Time': 0.3110},

    # Data for Intel Core i5-12400
    {'Processor': 'Intel Core i5-12400', 'Threads': 1, 'OM_Time': 228.78, 'SA_Time': 9.11589},
    {'Processor': 'Intel Core i5-12400', 'Threads': 2, 'OM_Time': 205.55, 'SA_Time': 3.1455},
    {'Processor': 'Intel Core i5-12400', 'Threads': 3, 'OM_Time': 207.25, 'SA_Time': 3.0223},
    {'Processor': 'Intel Core i5-12400', 'Threads': 4, 'OM_Time': 219.59, 'SA_Time': 2.79028},
    {'Processor': 'Intel Core i5-12400', 'Threads': 5, 'OM_Time': 219.04, 'SA_Time': 2.71200},
    {'Processor': 'Intel Core i5-12400', 'Threads': 6, 'OM_Time': 219.08, 'SA_Time': 2.197453},
    {'Processor': 'Intel Core i5-12400', 'Threads': 7, 'OM_Time': 210.11, 'SA_Time': 2.17898},
    {'Processor': 'Intel Core i5-12400', 'Threads': 8, 'OM_Time': 217.61, 'SA_Time': 2.03873},
    {'Processor': 'Intel Core i5-12400', 'Threads': 9, 'OM_Time': 225.80, 'SA_Time': 2.00546},
    {'Processor': 'Intel Core i5-12400', 'Threads': 10, 'OM_Time': 224.65, 'SA_Time': 1.90800},
    {'Processor': 'Intel Core i5-12400', 'Threads': 11, 'OM_Time': 207.40, 'SA_Time': 1.89340},
    {'Processor': 'Intel Core i5-12400', 'Threads': 12, 'OM_Time': 229.25, 'SA_Time': 1.83452},

    # Data for Intel Core i5-10400F
    {'Processor': 'Intel Core i5-10400F', 'Threads': 1, 'OM_Time': 279.456, 'SA_Time': 10.84313},
    {'Processor': 'Intel Core i5-10400F', 'Threads': 2, 'OM_Time': 291.123, 'SA_Time': 3.1490},
    {'Processor': 'Intel Core i5-10400F', 'Threads': 3, 'OM_Time': 288.789, 'SA_Time': 2.9650},
    {'Processor': 'Intel Core i5-10400F', 'Threads': 4, 'OM_Time': 294.001, 'SA_Time': 2.8585},
    {'Processor': 'Intel Core i5-10400F', 'Threads': 5, 'OM_Time': 280.555, 'SA_Time': 2.8417},
    {'Processor': 'Intel Core i5-10400F', 'Threads': 6, 'OM_Time': 292.345, 'SA_Time': 2.8274},
    {'Processor': 'Intel Core i5-10400F', 'Threads': 7, 'OM_Time': 285.678, 'SA_Time': 2.5675},
    {'Processor': 'Intel Core i5-10400F', 'Threads': 8, 'OM_Time': 290.987, 'SA_Time': 2.4387},
    {'Processor': 'Intel Core i5-10400F', 'Threads': 9, 'OM_Time': 270.234, 'SA_Time': 2.3254},
    {'Processor': 'Intel Core i5-10400F', 'Threads': 10, 'OM_Time': 287.890, 'SA_Time': 2.2180},
    {'Processor': 'Intel Core i5-10400F', 'Threads': 11, 'OM_Time': 293.456, 'SA_Time': 2.2008},
    {'Processor': 'Intel Core i5-10400F', 'Threads': 12, 'OM_Time': 286.789, 'SA_Time': 2.1399},

    # Data for Intel Xeon X5680
    {'Processor': 'Intel Xeon X5680', 'Threads': 1, 'OM_Time': 485.123, 'SA_Time': 32.2033},
    {'Processor': 'Intel Xeon X5680', 'Threads': 2, 'OM_Time': 501.789, 'SA_Time': 31.0291},
    {'Processor': 'Intel Xeon X5680', 'Threads': 3, 'OM_Time': 490.567, 'SA_Time': 31.4650},
    {'Processor': 'Intel Xeon X5680', 'Threads': 4, 'OM_Time': 483.999, 'SA_Time': 31.4585},
    {'Processor': 'Intel Xeon X5680', 'Threads': 5, 'OM_Time': 504.111, 'SA_Time': 31.8711},
    {'Processor': 'Intel Xeon X5680', 'Threads': 6, 'OM_Time': 496.222, 'SA_Time': 30.9563},
    {'Processor': 'Intel Xeon X5680', 'Threads': 7, 'OM_Time': 488.333, 'SA_Time': 30.0775},
    {'Processor': 'Intel Xeon X5680', 'Threads': 8, 'OM_Time': 499.444, 'SA_Time': 29.3552},
    {'Processor': 'Intel Xeon X5680', 'Threads': 9, 'OM_Time': 484.555, 'SA_Time': 29.1227},
    {'Processor': 'Intel Xeon X5680', 'Threads': 10, 'OM_Time': 503.666, 'SA_Time': 28.6135},
    {'Processor': 'Intel Xeon X5680', 'Threads': 11, 'OM_Time': 492.777, 'SA_Time': 28.4155},
    {'Processor': 'Intel Xeon X5680', 'Threads': 12, 'OM_Time': 487.888, 'SA_Time': 28.2818},

    # Data for Intel Core i7-8600U
    {'Processor': 'Intel Core i7-8600U', 'Threads': 1, 'OM_Time': 737.18, 'SA_Time': 15.4439},
    {'Processor': 'Intel Core i7-8600U', 'Threads': 2, 'OM_Time': 708.64, 'SA_Time': 5.6211},
    {'Processor': 'Intel Core i7-8600U', 'Threads': 3, 'OM_Time': 727.26, 'SA_Time': 5.2782},
    {'Processor': 'Intel Core i7-8600U', 'Threads': 4, 'OM_Time': 719.03, 'SA_Time': 4.9870},
    {'Processor': 'Intel Core i7-8600U', 'Threads': 5, 'OM_Time': 713.47, 'SA_Time': 4.8363},
    {'Processor': 'Intel Core i7-8600U', 'Threads': 6, 'OM_Time': 737.33, 'SA_Time': 4.7900},
    {'Processor': 'Intel Core i7-8600U', 'Threads': 7, 'OM_Time': 724.43, 'SA_Time': 4.7912},
    {'Processor': 'Intel Core i7-8600U', 'Threads': 8, 'OM_Time': 745.19, 'SA_Time': 4.3633},
    
    # Data for AMD Ryzen 5 7535HS
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 1, 'OM_Time': 337.085, 'SA_Time': 13.1583},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 2, 'OM_Time': 312.039, 'SA_Time': 3.6231},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 3, 'OM_Time': 322.966, 'SA_Time': 3.3233},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 4, 'OM_Time': 331.815, 'SA_Time': 2.7739},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 5, 'OM_Time': 323.198, 'SA_Time': 2.7235},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 6, 'OM_Time': 338.604, 'SA_Time': 2.6260},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 7, 'OM_Time': 318.339, 'SA_Time': 2.5235},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 8, 'OM_Time': 324.357, 'SA_Time': 2.45652},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 9, 'OM_Time': 313.659, 'SA_Time': 2.47323},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 10, 'OM_Time': 323.354, 'SA_Time': 2.5379},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 11, 'OM_Time': 316.371, 'SA_Time': 2.3661},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 12, 'OM_Time': 338.241, 'SA_Time': 2.31333},

    # Data for AMD Ryzen 5 7530U
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 1, 'OM_Time': 250.940, 'SA_Time': 12.999141},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 2, 'OM_Time': 255.135, 'SA_Time': 3.375539},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 3, 'OM_Time': 256.637, 'SA_Time': 3.275369},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 4, 'OM_Time': 255.057, 'SA_Time': 2.330764},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 5, 'OM_Time': 253.923, 'SA_Time': 2.853088},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 6, 'OM_Time': 249.392, 'SA_Time': 2.735806},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 7, 'OM_Time': 250.393, 'SA_Time': 2.56469},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 8, 'OM_Time': 255.239, 'SA_Time': 2.467956},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 9, 'OM_Time': 257.817, 'SA_Time': 2.423144},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 10, 'OM_Time': 259.677, 'SA_Time': 2.327546},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 11, 'OM_Time': 253.456, 'SA_Time': 2.704307},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 12, 'OM_Time': 250.298, 'SA_Time': 2.502371}
]

# Convert list of dictionaries to pandas DataFrame
df = pd.DataFrame(data)

# --- Color Coding (Global for consistency) ---
# Shades from indigo/dark blue to light blue
intel_color_palette = ["#00084D", "#0000FF", "#4989FF", "#1E9AFF", "#52A3C4", "#1BF0FF", '#007BA7', "#00BFFF"]
# Shades from burgundy/dark red to bright red/orange-red
amd_color_palette = ["#D3511E", "#FF0000", "#FF0055",]
# Fallback palette for others or if primary colors run out
try:
    fallback_palette = plt.get_cmap('tab20').colors
except AttributeError: # For older matplotlib versions
    fallback_palette = [plt.get_cmap('tab20')(i) for i in np.linspace(0, 1, 20)]

color_map_cache = {} # Cache assigned colors to maintain consistency across plots for the same processor

def get_processor_color(processor_name):
    """Gets a consistent color for a processor."""
    if processor_name in color_map_cache:
        return color_map_cache[processor_name]

    # Determine which palette to use and get the next color
    # This logic is simplified; for true global state across function calls,
    # you might need to manage indices outside or pass them.
    # For now, this ensures that within one script run, calls for the same processor get the same color.
    # A more robust solution would involve initializing a global color assignment at the script start.
    # However, for this script's flow, we'll re-initialize indices per plot type if needed,
    # but prioritize the cache.

    # Simplified: just pick from a combined pool if not Intel/AMD for fallback
    # This part of the color assignment logic is tricky to make globally unique without a class or global counters.
    # The current bar chart function re-calculates color_map each time.
    # For consistency across *different* plot function calls, we rely on the cache.
    if "Intel" in processor_name:
        # Find first unused Intel color
        for color in intel_color_palette:
            if color not in color_map_cache.values():
                color_map_cache[processor_name] = color
                return color
        # If all used, cycle
        color_map_cache[processor_name] = intel_color_palette[len(color_map_cache) % len(intel_color_palette)]
        return color_map_cache[processor_name]

    elif "AMD" in processor_name:
        for color in amd_color_palette:
            if color not in color_map_cache.values():
                color_map_cache[processor_name] = color
                return color
        color_map_cache[processor_name] = amd_color_palette[len(color_map_cache) % len(amd_color_palette)]
        return color_map_cache[processor_name]
    else:
        for color in fallback_palette:
            if color not in color_map_cache.values():
                color_map_cache[processor_name] = color
                return color
        color_map_cache[processor_name] = fallback_palette[len(color_map_cache) % len(fallback_palette)]
        return color_map_cache[processor_name]


def plot_processor_performance_bar(dataframe, processors_to_plot, time_metric):
    """
    Plots a bar chart of performance for selected processors and time metric.
    """
    if not isinstance(processors_to_plot, list):
        processors_to_plot = [processors_to_plot]

    plt.style.use('seaborn-v0_8-whitegrid')
    fig, ax = plt.subplots(figsize=(20, 12))

    relevant_data_for_threads = dataframe[dataframe['Processor'].isin(processors_to_plot)]
    if relevant_data_for_threads.empty:
        print(f"Warning: Data for the selected processors not found in DataFrame for bar chart.")
        ax.set_title(f"No data for {', '.join(processors_to_plot)}")
        plt.show()
        return

    all_thread_counts = sorted(relevant_data_for_threads['Threads'].unique())
    num_processors_in_group = len(processors_to_plot)
    if num_processors_in_group == 0:
        print("Warning: The list of processors to plot for bar chart is empty.")
        ax.set_title("Processor list is empty")
        plt.show()
        return

    # This local color_map is for the current bar plot instance.
    # We use get_processor_color to try and maintain consistency with other plots via color_map_cache
    local_color_map = {p_name: get_processor_color(p_name) for p_name in processors_to_plot}


    x_indices = np.arange(len(all_thread_counts))
    total_group_width = 0.8
    slot_width_per_bar = total_group_width / num_processors_in_group
    visual_bar_width = slot_width_per_bar * (0.7 if num_processors_in_group == 1 else 0.9)

    for i, processor_name in enumerate(processors_to_plot):
        processor_data = dataframe[dataframe['Processor'] == processor_name].sort_values(by='Threads')
        if not processor_data.empty:
            current_processor_times = []
            processor_data_indexed = processor_data.set_index('Threads')
            for tc in all_thread_counts:
                current_processor_times.append(processor_data_indexed.loc[tc, time_metric] if tc in processor_data_indexed.index else np.nan)
            
            offset = (i - num_processors_in_group / 2 + 0.5) * slot_width_per_bar
            bar_positions = x_indices + offset
            ax.bar(bar_positions, current_processor_times, width=visual_bar_width, label=processor_name, color=local_color_map.get(processor_name))
        else:
            print(f"Warning: Data for processor '{processor_name}' not found (skipping in bar chart).")

    ax.set_xlabel("Number of Threads", fontsize=14)
    ax.set_ylabel(f"{time_metric.replace('_', ' ')} (seconds)", fontsize=14)
    ax.set_title(f"Processor Performance : {time_metric.replace('_', ' ')} ", fontsize=16)
    ax.set_xticks(x_indices)
    ax.set_xticklabels(all_thread_counts, fontsize=12)
    ax.tick_params(axis='y', labelsize=12)
    if len(x_indices) > 0:
        ax.set_xlim(x_indices[0] - 0.5, x_indices[-1] + 0.5)
    ax.legend(title="Processor", loc='upper right', fontsize=12, title_fontsize=14, fancybox=True, framealpha=0.9, facecolor='whitesmoke', edgecolor='black')
    plt.grid(True, axis='y', linestyle='--', alpha=0.7)
    plt.tight_layout()
    plt.show()


def plot_processor_performance_line(dataframe, processors_to_plot, time_metric):
    """
    Plots a line chart of performance for selected processors and time metric.
    """
    if not isinstance(processors_to_plot, list):
        processors_to_plot = [processors_to_plot]

    plt.style.use('seaborn-v0_8-whitegrid')
    plt.figure(figsize=(15, 8)) # Adjusted figure size for line chart

    max_threads_for_plot = 0

    for processor_name in processors_to_plot:
        processor_data = dataframe[dataframe['Processor'] == processor_name].sort_values(by='Threads')
        if not processor_data.empty:
            plt.plot(processor_data['Threads'], processor_data[time_metric], marker='o', linestyle='-', label=processor_name, color=get_processor_color(processor_name))
            current_max_threads = processor_data['Threads'].max()
            if current_max_threads > max_threads_for_plot:
                max_threads_for_plot = current_max_threads
        else:
            print(f"Warning: Data for processor '{processor_name}' not found (skipping in line chart).")

    plt.xlabel("Number of Threads", fontsize=14)
    plt.ylabel(f"{time_metric.replace('_', ' ')} (seconds)", fontsize=14)
    plt.title(f"Processor Performance (Line Chart): {time_metric.replace('_', ' ')} ", fontsize=16)
    
    if max_threads_for_plot > 0:
        # Ensure x-axis ticks are sensible for thread counts
        tick_values = sorted(dataframe[dataframe['Processor'].isin(processors_to_plot)]['Threads'].unique())
        if len(tick_values) > 20: # Limit number of ticks if too many unique thread counts
             step = len(tick_values) // 15
             tick_values = tick_values[::step] if step > 0 else tick_values

        plt.xticks(tick_values, fontsize=12)
        plt.xlim(left=0.5, right=max_threads_for_plot + 0.5)
    
    plt.legend(title="Processor", loc='upper right', fontsize=12, title_fontsize=14, fancybox=True, framealpha=0.9, facecolor='whitesmoke', edgecolor='black')
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.tight_layout()
    plt.show()


def plot_single_processor_om_vs_sa(dataframe, processor_name):
    """
    Plots OM_Time vs SA_Time for a single processor on the same graph.
    Uses a secondary y-axis for SA_Time if scales differ significantly.
    """
    processor_data = dataframe[dataframe['Processor'] == processor_name].sort_values(by='Threads')
    if processor_data.empty:
        print(f"Warning: Data for processor '{processor_name}' not found for OM vs SA plot.")
        return

    fig, ax1 = plt.subplots(figsize=(15, 8))
    plt.style.use('seaborn-v0_8-whitegrid')

    color_om = get_processor_color(f"{processor_name}_OM") # Get a distinct color variation
    ax1.set_xlabel('Number of Threads', fontsize=14)
    ax1.set_ylabel('OM_Time (seconds)', color=color_om, fontsize=14)
    ax1.plot(processor_data['Threads'], processor_data['OM_Time'], color=color_om, marker='o', linestyle='-', label='OM_Time')
    ax1.tick_params(axis='y', labelcolor=color_om, labelsize=12)
    ax1.tick_params(axis='x', labelsize=12)

    # Determine if secondary axis is needed
    om_max = processor_data['OM_Time'].max()
    sa_max = processor_data['SA_Time'].max()

    # Heuristic: if one metric is more than 5-10x the other, use secondary axis
    if om_max / sa_max > 10 or sa_max / om_max > 10 :
        ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis
        color_sa = get_processor_color(f"{processor_name}_SA") # Distinct color
        ax2.set_ylabel('SA_Time (seconds)', color=color_sa, fontsize=14)
        ax2.plot(processor_data['Threads'], processor_data['SA_Time'], color=color_sa, marker='x', linestyle='--', label='SA_Time')
        ax2.tick_params(axis='y', labelcolor=color_sa, labelsize=12)
        fig.tight_layout() # otherwise the right y-label is slightly clipped
         # Adding combined legend
        lines, labels = ax1.get_legend_handles_labels()
        lines2, labels2 = ax2.get_legend_handles_labels()
        ax2.legend(lines + lines2, labels + labels2, loc='upper center', fontsize=12, title_fontsize=14, fancybox=True, framealpha=0.9, facecolor='whitesmoke', edgecolor='black', title=processor_name)

    else: # Plot on the same axis if scales are similar
        color_sa = get_processor_color(f"{processor_name}_SA")
        ax1.plot(processor_data['Threads'], processor_data['SA_Time'], color=color_sa, marker='x', linestyle='--', label='SA_Time')
        ax1.legend(loc='upper center', fontsize=12, title_fontsize=14, fancybox=True, framealpha=0.9, facecolor='whitesmoke', edgecolor='black', title=processor_name)


    plt.title(f'OM_Time vs SA_Time for {processor_name}', fontsize=16)
    
    tick_values = sorted(processor_data['Threads'].unique())
    if len(tick_values) > 20:
         step = len(tick_values) // 15
         tick_values = tick_values[::step] if step > 0 else tick_values
    ax1.set_xticks(tick_values)

    plt.grid(True, linestyle='--', alpha=0.7)
    plt.show()


def plot_specific_thread_performance_bar(dataframe, thread_focus='single', time_metrics=None):
    """
    Plots a bar chart comparing processors at a specific thread count (single or max).
    Args:
        dataframe (pd.DataFrame): The main data.
        thread_focus (str): 'single' for 1-thread, 'max' for max threads per CPU.
        time_metrics (list): List of time metrics to plot, e.g., ['OM_Time', 'SA_Time'].
    """
    if time_metrics is None:
        time_metrics = ['OM_Time', 'SA_Time']

    plt.style.use('seaborn-v0_8-whitegrid')
    fig, ax = plt.subplots(figsize=(20, 12))

    plot_data = []
    processor_order = sorted(dataframe['Processor'].unique())

    for processor_name in processor_order:
        specific_proc_data = dataframe[dataframe['Processor'] == processor_name]
        if specific_proc_data.empty:
            continue

        if thread_focus == 'single':
            data_point = specific_proc_data[specific_proc_data['Threads'] == 1]
            if data_point.empty: # Skip if no 1-thread data
                print(f"Warning: No 1-thread data for {processor_name} (skipping in specific thread plot).")
                continue
        elif thread_focus == 'max':
            data_point = specific_proc_data[specific_proc_data['Threads'] == specific_proc_data['Threads'].max()]
            if data_point.empty: # Should not happen if processor has data
                continue
        else:
            print(f"Warning: Invalid thread_focus '{thread_focus}'. Use 'single' or 'max'.")
            return
        
        # Take the first row if multiple exist for max threads (e.g. if data is duplicated)
        data_point = data_point.iloc[0] 
        
        row = {'Processor': processor_name}
        for metric in time_metrics:
            row[metric] = data_point[metric]
        plot_data.append(row)

    if not plot_data:
        print(f"Warning: No data to plot for {thread_focus}-thread performance.")
        ax.set_title(f"No data for {thread_focus}-thread performance")
        plt.show()
        return

    plot_df = pd.DataFrame(plot_data)
    
    num_metrics = len(time_metrics)
    x_indices = np.arange(len(plot_df['Processor']))
    total_group_width = 0.8
    bar_width = total_group_width / num_metrics
    
    metric_colors = [get_processor_color(f"Metric_{i}") for i in range(num_metrics)] # Simple distinct colors for metrics

    for i, metric in enumerate(time_metrics):
        offset = (i - num_metrics / 2 + 0.5) * bar_width
        bar_positions = x_indices + offset
        ax.bar(bar_positions, plot_df[metric], width=bar_width * 0.9, label=metric.replace('_',' '), color=metric_colors[i])

    ax.set_xlabel("Processor", fontsize=14)
    ax.set_ylabel("Time (seconds)", fontsize=14)
    title_str = "Single-Thread (1T)" if thread_focus == 'single' else "Max-Thread"
    ax.set_title(f"{title_str} Performance Comparison", fontsize=16)
    
    ax.set_xticks(x_indices)
    ax.set_xticklabels(plot_df['Processor'], rotation=45, ha="right", fontsize=10)
    ax.tick_params(axis='y', labelsize=12)

    ax.legend(title="Time Metric", loc='upper right', fontsize=12, title_fontsize=14, fancybox=True, framealpha=0.9, facecolor='whitesmoke', edgecolor='black')
    plt.grid(True, axis='y', linestyle='--', alpha=0.7)
    plt.tight_layout()
    plt.show()


# --- Usage Examples ---
if __name__ == "__main__":
    all_processors = df['Processor'].unique().tolist()
    # Sort processors for consistent plotting order, e.g., alphabetically or by a performance metric if desired
    all_processors.sort() 

    # --- Bar Charts (Existing) ---
    print("Plotting OM_Time for all processors (Bar Chart)...")
    plot_processor_performance_bar(df, all_processors, 'OM_Time')

    print("Plotting SA_Time for all processors (Bar Chart)...")
    plot_processor_performance_bar(df, all_processors, 'SA_Time')

    intel_processors = sorted(['Intel Core i9-14900K', 'Intel Core i9-13900K', 'Intel Core i5-12400', 'Intel Core i5-10400F', 'Intel Xeon X5680'])
    print(f"Plotting OM_Time for {', '.join(intel_processors)} (Bar Chart)...")
    plot_processor_performance_bar(df, intel_processors, 'OM_Time')

    amd_processors = sorted(['AMD Ryzen 5 7535HS', 'AMD Ryzen 5 7530U'])
    print(f"Plotting SA_Time for {', '.join(amd_processors)} (Bar Chart)...")
    plot_processor_performance_bar(df, amd_processors, 'SA_Time')
    
    # --- Line Charts (New) ---
    print("Plotting OM_Time for all processors (Line Chart)...")
    plot_processor_performance_line(df, all_processors, 'OM_Time')

    print("Plotting SA_Time for all processors (Line Chart)...")
    plot_processor_performance_line(df, all_processors, 'SA_Time')

    print(f"Plotting OM_Time for {', '.join(intel_processors)} (Line Chart)...")
    plot_processor_performance_line(df, intel_processors, 'OM_Time')

    print(f"Plotting SA_Time for {', '.join(amd_processors)} (Line Chart)...")
    plot_processor_performance_line(df, amd_processors, 'SA_Time')

    # --- Single Processor OM_Time vs SA_Time (New) ---
    representative_processor = 'Intel Core i9-14900K'
    print(f"Plotting OM_Time vs SA_Time for {representative_processor}...")
    plot_single_processor_om_vs_sa(df, representative_processor)
    
    representative_processor_amd = 'AMD Ryzen 5 7535HS'
    print(f"Plotting OM_Time vs SA_Time for {representative_processor_amd}...")
    plot_single_processor_om_vs_sa(df, representative_processor_amd)


    # --- Specific Thread Performance (New) ---
    print("Plotting Single-Thread (1T) performance for all processors (OM_Time & SA_Time)...")
    plot_specific_thread_performance_bar(df, thread_focus='single', time_metrics=['OM_Time', 'SA_Time'])

    print("Plotting Max-Thread performance for all processors (OM_Time & SA_Time)...")
    plot_specific_thread_performance_bar(df, thread_focus='max', time_metrics=['OM_Time', 'SA_Time'])
    
    # Example with a custom selection for bar chart
    custom_comparison = sorted(['Intel Core i9-14900K', 'AMD Ryzen 5 7535HS', 'Intel Core i5-12400'])
    print(f"Plotting OM_Time for {', '.join(custom_comparison)} (Bar Chart)...")
    plot_processor_performance_bar(df, custom_comparison, 'OM_Time')
    
    # Example with a larger number of processors for line chart to test color and width scalability
    comparison_large_intel = sorted(['Intel Core i9-14900K', 'Intel Core i9-13900K', 'Intel Core i5-12400', 'Intel Core i5-10400F', 'Intel Xeon X5680', 'Intel Core i7-8600U'])
    print(f"Plotting OM_Time for a large set of Intel processors (Line Chart)...")
    plot_processor_performance_line(df, comparison_large_intel, 'OM_Time')

