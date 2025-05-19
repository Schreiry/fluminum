import matplotlib.pyplot as plt
import csv

# Пример данных в виде списка словарей (можно заменить на чтение из CSV)
DEFAULT_DATA = [
    
    
    # iNTEL CORE i9-13900K : 
    
    {'Processor': 'Intel Core i9-13900K', 'Threads': 1, 'OM_Time': 178.498, 'SA_Time': 7.6108},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 2, 'OM_Time': 178.498, 'SA_Time': 1.7591},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 3, 'OM_Time': 178.498, 'SA_Time': 1.7450},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 4, 'OM_Time': 178.498, 'SA_Time': 1.3633},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 5, 'OM_Time': 178.498, 'SA_Time': 1.1606},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 6, 'OM_Time': 178.498, 'SA_Time': 1.0053},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 7, 'OM_Time': 178.498, 'SA_Time': 1.0019},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 8, 'OM_Time': 178.498, 'SA_Time': 0.9992},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 9, 'OM_Time': 178.498, 'SA_Time': 0.9981},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 10, 'OM_Time': 178.498, 'SA_Time': 0.9709},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 11, 'OM_Time': 178.498, 'SA_Time': 0.9746},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 12, 'OM_Time': 178.498, 'SA_Time': 0.9603},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 13, 'OM_Time': 178.498, 'SA_Time': 0.9578},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 14, 'OM_Time': 178.498, 'SA_Time': 0.9528},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 15, 'OM_Time': 178.498, 'SA_Time': 0.9478},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 16, 'OM_Time': 178.498, 'SA_Time': 0.9428},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 17, 'OM_Time': 178.498, 'SA_Time': 0.8903},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 18, 'OM_Time': 178.498, 'SA_Time': 0.8778},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 19, 'OM_Time': 178.498, 'SA_Time': 0.8553},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 20, 'OM_Time': 178.498, 'SA_Time': 0.8328},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 21, 'OM_Time': 178.498, 'SA_Time': 0.6303},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 22, 'OM_Time': 178.498, 'SA_Time': 0.6102},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 23, 'OM_Time': 178.498, 'SA_Time': 0.5891},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 24, 'OM_Time': 178.498, 'SA_Time': 0.5230},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 25, 'OM_Time': 178.498, 'SA_Time': 0.5115},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 26, 'OM_Time': 178.498, 'SA_Time': 0.5000},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 27, 'OM_Time': 178.498, 'SA_Time': 0.4885},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 28, 'OM_Time': 178.498, 'SA_Time': 0.4170},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 29, 'OM_Time': 178.498, 'SA_Time': 0.4055},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 30, 'OM_Time': 178.498, 'SA_Time': 0.3540},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 31, 'OM_Time': 178.498, 'SA_Time': 0.3425},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 32, 'OM_Time': 178.498, 'SA_Time': 0.3110},
    
    
    
        # iNTEL CORE i5-12400 :
    {'Processor': 'Intel Core i5-12400', 'Threads': 1, 'OM_Time': 220.85, 'SA_Time': 11.84313},
    {'Processor': 'Intel Core i5-12400', 'Threads': 2, 'OM_Time': 220.85, 'SA_Time': 3.1455},
    {'Processor': 'Intel Core i5-12400', 'Threads': 3, 'OM_Time': 220.85, 'SA_Time': 3.0223},
    {'Processor': 'Intel Core i5-12400', 'Threads': 4, 'OM_Time': 220.85, 'SA_Time': 2.79028},
    {'Processor': 'Intel Core i5-12400', 'Threads': 5, 'OM_Time': 220.85, 'SA_Time': 2.71200},
    {'Processor': 'Intel Core i5-12400', 'Threads': 6, 'OM_Time': 220.85, 'SA_Time': 2.197453},
    {'Processor': 'Intel Core i5-12400', 'Threads': 7, 'OM_Time': 220.85, 'SA_Time': 2.17898},
    {'Processor': 'Intel Core i5-12400', 'Threads': 8, 'OM_Time': 220.85, 'SA_Time': 2.03873},
    {'Processor': 'Intel Core i5-12400', 'Threads': 9, 'OM_Time': 220.85, 'SA_Time': 2.00546},
    {'Processor': 'Intel Core i5-12400', 'Threads': 10, 'OM_Time': 220.85, 'SA_Time': 1.90800},
    {'Processor': 'Intel Core i5-12400', 'Threads': 11, 'OM_Time': 220.85, 'SA_Time': 1.89340},
    {'Processor': 'Intel Core i5-12400', 'Threads': 12, 'OM_Time': 220.85, 'SA_Time': 1.83452},
    
    
    
    # iNTEL CORE i7-8600U : 
    {'Processor': 'Intel Core i7-8600U', 'Threads': 1, 'OM_Time': 712.19, 'SA_Time': 15.4439},
    {'Processor': 'Intel Core i7-8600U', 'Threads': 2, 'OM_Time': 712.19, 'SA_Time': 5.6211},
    {'Processor': 'Intel Core i7-8600U', 'Threads': 3, 'OM_Time': 712.19, 'SA_Time': 5.2782},
    {'Processor': 'Intel Core i7-8600U', 'Threads': 4, 'OM_Time': 712.19, 'SA_Time': 4.9870},
    {'Processor': 'Intel Core i7-8600U', 'Threads': 5, 'OM_Time': 712.19, 'SA_Time': 4.8363},
    {'Processor': 'Intel Core i7-8600U', 'Threads': 6, 'OM_Time': 712.19, 'SA_Time': 4.7900},
    {'Processor': 'Intel Core i7-8600U', 'Threads': 7, 'OM_Time': 712.19, 'SA_Time': 4.7912},
    {'Processor': 'Intel Core i7-8600U', 'Threads': 8, 'OM_Time': 712.19, 'SA_Time': 4.3633},
    
    
    
    
    # AMD RYZEN 5 7535HS : 
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 1, 'OM_Time': 335.483, 'SA_Time': 13.1583},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 2, 'OM_Time': 335.483, 'SA_Time': 3.6231},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 3, 'OM_Time': 335.483, 'SA_Time': 3.3233},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 4, 'OM_Time': 335.483, 'SA_Time': 2.7739},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 5, 'OM_Time': 335.483, 'SA_Time': 2.7235},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 6, 'OM_Time': 335.483, 'SA_Time': 2.6260},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 7, 'OM_Time': 335.483, 'SA_Time': 2.5235},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 8, 'OM_Time': 335.483, 'SA_Time': 2.45652},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 9, 'OM_Time': 335.483, 'SA_Time': 2.47323},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 10, 'OM_Time': 335.483, 'SA_Time': 2.5379},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 11, 'OM_Time': 335.4834, 'SA_Time': 2.3661},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 12, 'OM_Time': 335.483, 'SA_Time': 2.31333},
    
    
        # AMD RYZEN 5 7530U : 
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 1, 'OM_Time': 252.505, 'SA_Time': 12.999141},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 2, 'OM_Time': 252.505, 'SA_Time': 3.375539},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 3, 'OM_Time': 252.505, 'SA_Time': 3.275369},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 4, 'OM_Time': 252.505, 'SA_Time': 2.330764},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 5, 'OM_Time': 252.505, 'SA_Time': 2.853088},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 6, 'OM_Time': 252.505, 'SA_Time': 2.735806},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 7, 'OM_Time': 252.505, 'SA_Time': 2.56469},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 8, 'OM_Time': 252.505, 'SA_Time': 2.467956},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 9, 'OM_Time': 252.505, 'SA_Time': 2.423144},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 10, 'OM_Time': 252.505, 'SA_Time': 2.327546},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 11, 'OM_Time': 252.505, 'SA_Time': 2.704307},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 12, 'OM_Time': 252.505, 'SA_Time': 2.502371},
    
    
    
]

def load_data_from_csv(filepath):
    """
    Читает данные из CSV-файла.
    Ожидаемые колонки: Processor,Threads,OM_Time,SA_Time
    """
    data = []
    try:
        with open(filepath, mode='r', encoding='utf-8') as csvfile:
            reader = csv.DictReader(csvfile)
            for row in reader:
                try:
                    data.append({
                        'Processor': row['Processor'],
                        'Threads': int(row['Threads']),
                        'OM_Time': float(row['OM_Time']),
                        'SA_Time': float(row['SA_Time'])
                    })
                except ValueError as e:
                    print(f"Ошибка конвертации данных в строке: {row}. Ошибка: {e}")
                    continue 
                except KeyError as e:
                    print(f"Отсутствует необходимая колонка: {e} в файле {filepath}. Строка: {row}")
                    return [] 
        return data
    except FileNotFoundError:
        print(f"Файл не найден: {filepath}")
        return []
    except Exception as e:
        print(f"Произошла ошибка при чтении CSV файла: {e}")
        return []

def preprocess_data(raw_data_list):
    """
    Группирует данные по процессорам и сортирует по количеству потоков.
    """
    processed_data = {}
    for record in raw_data_list:
        processor = record['Processor']
        if processor not in processed_data:
            processed_data[processor] = {'Threads': [], 'OM_Time': [], 'SA_Time': []}
        
        processed_data[processor]['Threads'].append(record['Threads'])
        processed_data[processor]['OM_Time'].append(record['OM_Time'])
        processed_data[processor]['SA_Time'].append(record['SA_Time'])

    for processor in processed_data:
        sorted_points = sorted(zip(
            processed_data[processor]['Threads'],
            processed_data[processor]['OM_Time'],
            processed_data[processor]['SA_Time']
        ))
        processed_data[processor]['Threads'] = [p[0] for p in sorted_points]
        processed_data[processor]['OM_Time'] = [p[1] for p in sorted_points]
        processed_data[processor]['SA_Time'] = [p[2] for p in sorted_points]
        
    return processed_data

def plot_performance(data_by_processor, output_filename="performance_comparison.png", dpi=300):
    """
    Строит и сохраняет график производительности.
    """
    if not data_by_processor:
        print("Нет данных для построения графика.")
        return

    fig, ax = plt.subplots(figsize=(14, 8)) 

    # --- Начало изменений для цветов ---
    # Палитры цветов:
    # Intel: от индиго/темно-синего до ярко-голубого
    intel_palette = ['#081040', '#0123fa', '#1E88E5', '#0df0ff', '#05a697', '#00709b', '#81e0ff']
    # AMD Ryzen: от бордового до красного
    amd_palette = ['#540909', '#c12248', '#ff003e', '#ff0600', '#ff5900', '#EF5350', '#b70056']
    # Резервные цвета для других производителей (например, зеленые, оранжевые, фиолетовые)
    other_palette_options = ['#2E8B57', '#FF8C00', '#9932CC', '#008B8B', '#8FBC8F', '#D2691E']

    intel_color_idx = 0
    amd_color_idx = 0
    other_color_idx = 0
    # --- Конец изменений для цветов ---

    max_overall_speedup = 0
    max_speedup_annotation_info = None

    for processor_name, p_data in data_by_processor.items():
        # --- Логика выбора цвета ---
        assigned_color = None
        if "Intel" in processor_name:
            assigned_color = intel_palette[intel_color_idx % len(intel_palette)]
            intel_color_idx += 1
        elif "AMD" in processor_name or "Ryzen" in processor_name: # "Ryzen" более специфичен для AMD
            assigned_color = amd_palette[amd_color_idx % len(amd_palette)]
            amd_color_idx += 1
        else:
            assigned_color = other_palette_options[other_color_idx % len(other_palette_options)]
            other_color_idx += 1
        # --- Конец логики выбора цвета ---

        threads = p_data['Threads']
        om_times = p_data['OM_Time']
        sa_times = p_data['SA_Time']

        if not threads: 
            print(f"Нет данных о потоках для процессора {processor_name}")
            continue

        ax.plot(threads, om_times, linestyle='-', marker='o', 
                label=f'{processor_name} - OM', color=assigned_color) # Используем assigned_color

        ax.plot(threads, sa_times, linestyle='--', marker='x', 
                label=f'{processor_name} - SA', color=assigned_color) # Используем assigned_color

        for j, thread_count in enumerate(threads):
            om_time = om_times[j]
            sa_time = sa_times[j]
            if sa_time > 0 and om_time > 0: 
                current_speedup = om_time / sa_time
                if current_speedup > max_overall_speedup:
                    max_overall_speedup = current_speedup
                    max_speedup_annotation_info = {
                        'processor': processor_name,
                        'threads': thread_count,
                        'om_time': om_time,
                        'sa_time': sa_time,
                        'speedup': current_speedup,
                        'color': assigned_color # Используем assigned_color для аннотации
                    }
    
    ax.set_title('OM-ისა და SA-ს მუშაობის შედარება სხვადასხვა პროცესორებზე', fontsize=16) # Сравнение производительности OM vs SA на разных процессорах (груз.)
    ax.set_xlabel('ნაკადების რაოდენობა', fontsize=14) # Число потоков (груз.)
    ax.set_ylabel('შესრულების დრო, წ', fontsize=14) # Время выполнения, с (груз.)
    
    all_threads_values = sorted(list(set(t for p_data in data_by_processor.values() for t in p_data['Threads'])))
    if all_threads_values:
        ax.set_xticks(all_threads_values)
        # Для большого количества потоков можно настроить тики реже:
        if len(all_threads_values) > 15: # Например, если больше 15 уникальных значений потоков
             from matplotlib.ticker import MaxNLocator
             ax.xaxis.set_major_locator(MaxNLocator(nbins=15, integer=True))


    ax.legend(fontsize=10, title="პროცესორი და ალგორითმი") # Процессор и Алгоритм (груз.)
    ax.grid(True, which="both", ls="-", alpha=0.5)

    if max_speedup_annotation_info:
        info = max_speedup_annotation_info
        annotation_text = (f"მაქს. აჩქარება SA: {info['speedup']:.2f}x\n" # Макс. ускорение SA (груз.)
                           f"პროცესორი: {info['processor']}\n" # Процессор (груз.)
                           f"ნაკადები: {info['threads']}\n" # Потоки (груз.)
                           f"(OM: {info['om_time']:.2f}s, SA: {info['sa_time']:.2f}s)")
        
        y_annotate_pos = info['sa_time'] 
        
        # Небольшая корректировка позиции текста аннотации, чтобы он не вылезал за пределы графика
        x_text_offset = info['threads'] + (ax.get_xlim()[1] - ax.get_xlim()[0]) * 0.02
        y_text_offset = y_annotate_pos + (ax.get_ylim()[1] - ax.get_ylim()[0]) * 0.05

        # Проверка, чтобы аннотация не уходила слишком вправо или вверх
        if x_text_offset > ax.get_xlim()[1] * 0.85 : # если текст уходит за 85% ширины
            x_text_offset = info['threads'] - (ax.get_xlim()[1] - ax.get_xlim()[0]) * 0.2 # сместить влево

        if y_text_offset > ax.get_ylim()[1] * 0.85: # если текст уходит за 85% высоты
             y_text_offset = y_annotate_pos - (ax.get_ylim()[1] - ax.get_ylim()[0]) * 0.1 # сместить вниз


        ax.annotate(annotation_text,
                    xy=(info['threads'], y_annotate_pos),
                    xytext=(x_text_offset, y_text_offset), 
                    arrowprops=dict(facecolor=info['color'], shrink=0.05, width=1, headwidth=5),
                    bbox=dict(boxstyle="round,pad=0.3", fc="white", ec=info['color'], alpha=0.9),
                    fontsize=9,
                    color='black' 
                   )

    try:
        plt.savefig(output_filename, dpi=dpi, bbox_inches='tight')
        print(f"График сохранен в файл: {output_filename} (DPI: {dpi})")
    except Exception as e:
        print(f"Ошибка при сохранении графика: {e}")

    plt.show()

def main():
    """
    Основная функция для выполнения скрипта.
    """
    # Вариант 1: Использование данных по умолчанию (список словарей)
    # raw_data = DEFAULT_DATA
    # print("Используются данные по умолчанию.")

    # Вариант 2: Чтение данных из CSV-файла
    csv_filename = "performance_data.csv"
    header = ['Processor', 'Threads', 'OM_Time', 'SA_Time']
    with open(csv_filename, mode='w', newline='', encoding='utf-8') as f:
        writer = csv.DictWriter(f, fieldnames=header)
        writer.writeheader()
        writer.writerows(DEFAULT_DATA)
    print(f"Пример CSV файла '{csv_filename}' создан с данными по умолчанию.")
    
    raw_data = load_data_from_csv(csv_filename)

    if not raw_data:
        print("Не удалось загрузить данные. Завершение работы.")
        return

    data_for_plot = preprocess_data(raw_data)
    
    if not data_for_plot:
        print("Нет данных после предобработки. Завершение работы.")
        return

    plot_performance(data_for_plot, output_filename="comparison_OM_vs_SA_custom_colors.png", dpi=300)

if __name__ == '__main__':
    main()