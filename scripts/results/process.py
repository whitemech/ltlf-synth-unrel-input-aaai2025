import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
from scipy.stats import wilcoxon
from brokenaxes import brokenaxes
import re 
from scipy.stats import friedmanchisquare
# Define file paths for the three algorithms
file_paths = {
    
    'direct': 'results-direct.csv',
    'belief': 'results-belief.csv',
    'mso': 'results-mso.csv'
}

# Converts to a test - group
def get_test_group(test_name):
    if 'tests/hiker' in test_name:
        return 'hiker'
    elif 'tests/sheep' in test_name:
        return 'sheep'
    elif 'tests/trap' in test_name:
        return 'trap'
    else:
        return 'other'

# Initialize a dictionary to hold dataframes
data = {}
timeout_value = 1500

def set_size(width, fraction=1, subplots=(1, 1)):
    """Set figure dimensions to avoid scaling in LaTeX.

    Parameters
    ----------
    width: float or string
            Document width in points, or string of predined document type
    fraction: float, optional
            Fraction of the width which you wish the figure to occupy
    subplots: array-like, optional
            The number of rows and columns of subplots.
    Returns
    -------
    fig_dim: tuple
            Dimensions of figure in inches
    """
    if width == 'aaai':
        width_pt = 504.0
    elif width == 'beamer':
        width_pt = 307.28987
    else:
        width_pt = width

    # Width of figure (in pts)
    fig_width_pt = width_pt * fraction
    # Convert from pt to inches
    inches_per_pt = 1 / 72.27

    # Golden ratio to set aesthetic figure height
    # https://disq.us/p/2940ij3
    golden_ratio = (5**.5 - 1) / 2

    # Figure width in inches
    fig_width_in = fig_width_pt * inches_per_pt
    # Figure height in inches
    fig_height_in = fig_width_in * golden_ratio * (subplots[0] / subplots[1])

    return (fig_width_in, fig_height_in)

def alphanum_key(s):
    """Turn a string into a list of string and number chunks.
    """
    return [int(text) if text.isdigit() else text for text in re.split('([0-9]+)', s)]

# Read each CSV file and store the dataframe in the dictionary
for algorithm, file_path in file_paths.items():
    print("Processing " + str(algorithm))
    df = pd.read_csv(file_path, dtype={"errmsg": "str"}, header=None, names=['Test Name', "Full Time (ms)", "err"])
    # Calculate the proportion of timeouts
    timeout_proportion = (df["Full Time (ms)"] < 0).mean()
    df['timeout_proportion'] = timeout_proportion
    df["Full Time (ms)"].fillna(0, inplace=True)
    df['complete time'] = df["Full Time (ms)"].apply(lambda x: timeout_value if x < 0 else x)
    df['test_group'] = df['Test Name'].apply(get_test_group)
    data[algorithm] = df
    print(data[algorithm])

# Get a comprehensive list of all test names
all_test_names = pd.concat([df['Test Name'] for df in data.values()]).unique()

# Reindex dataframes to ensure all test names are included
for algorithm, df in data.items():
    df = df.set_index('Test Name').reindex(all_test_names).reset_index()
    df['complete time'] = df['complete time'].fillna(0)
    df['test_group'] = df['test_group'].fillna(df['Test Name'].apply(get_test_group))
    data[algorithm] = df

def calculate_smallest_bar(data, group_name, min_value):
    smallest_bar = float('inf')
    
    for algorithm, df in data.items():
        group_data = df[df['test_group'] == group_name]
        for index, row in group_data.iterrows():
            if row['complete time'] > min_value:
                smallest_bar = min(smallest_bar, row['complete time'])
    
    if smallest_bar == float('inf'):
        return None  # or some default value if no bar is found above min_value
    return smallest_bar


# Define a function to generate and save plots for each group
def plot_group(group_name, group_df):
    
    
    group_df['Test Name'] = group_df['Test Name'].apply(lambda x: x.split('/')[-1].replace('.ltlf', ''))
    group_df = group_df.sort_values(by='Test Name', key=lambda col: col.map(alphanum_key))
    print(group_df)

    # Define the positions after sorting
    positions = np.arange(len(group_df))

    # Create figure and subplots
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=set_size("aaai", 1, (2, 1)), sharex=True, gridspec_kw={'height_ratios': [1, 4]})
    bar_width = 0.25

    # Define the y-limits for the broken axis
    if group_name == 'hiker':
        ylim1 = (0, 50)
        # Find the smallest value in the dataset that is bigger than ylim1[1]
        # Adjust ylim2 to be slightly lower than the smallest bar
        ylim2 = (calculate_smallest_bar(data, group_name, ylim1[1]) - 5 , max(group_df['complete time'].max(), timeout_value)+5)
    elif group_name == 'sheep':
        ylim1 = (0, 4)
        ylim2 = (6, max(group_df['complete time'].max(), timeout_value)+5)
    elif group_name == 'trap':
        ylim1 = (0, 3)
        ylim2 = (30, 50)
    
        

    # Plot the timeout line in both axis segments
    ax1.axhline(y=timeout_value, color='r', linestyle='--', label='Timeout', zorder=1)
    #ax2.axhline(y=timeout_value, color='r', linestyle='--', label='Timeout', zorder=1)

    # Plot data for each algorithm
    colors = sns.color_palette("bright", len(data))  # Using Seaborn color palette
    for i, (algorithm, df) in enumerate(data.items()):
        group_data = df[df['test_group'] == group_name].sort_values(by='Test Name', key=lambda col: col.map(alphanum_key))
        
        # Create an array to store positions for the current algorithm
        current_positions = positions + i * bar_width
        
        for j, (index, row) in enumerate(group_data.iterrows()):
            if row['complete time'] >= timeout_value:
                ax1.bar(current_positions[j], row['complete time'], width=bar_width, 
                        label=f"{algorithm} (timeout)" if j == 0 else "", color=colors[i], alpha=0.5, hatch='//', edgecolor="white", zorder=2)
                ax2.bar(current_positions[j], row['complete time'], width=bar_width, 
                        label=f"{algorithm} (timeout)" if j == 0 else "", color=colors[i], alpha=0.5, hatch='//', edgecolor="white", zorder=2)
            else:
                ax1.bar(current_positions[j], row['complete time'], width=bar_width, 
                        label=f"{algorithm}" if j == 0 else "", color=colors[i], zorder=2)
                ax2.bar(current_positions[j], row['complete time'], width=bar_width, 
                        label=f"{algorithm}" if j == 0 else "", color=colors[i], zorder=2)

    # Set limits and hide the spines between ax1 and ax2
    ax1.set_ylim(ylim2)
    ax2.set_ylim(ylim1)
    ax1.spines['bottom'].set_visible(False)
    ax2.spines['top'].set_visible(False)
    ax1.xaxis.tick_top()
    ax1.tick_params(labeltop=False)  # don't put tick labels at the top
    ax2.xaxis.tick_bottom()

    # Add diagonal lines to indicate the break
    d = .015  # how big to make the diagonal lines in axes coordinates
    kwargs = dict(transform=ax1.transAxes, color='k', clip_on=False)
    ax1.plot((-d, +d), (-d, +d), **kwargs)  # bottom-left diagonal
    ax1.plot((1 - d, 1 + d), (-d, +d), **kwargs)  # bottom-right diagonal

    kwargs.update(transform=ax2.transAxes)
    ax2.plot((-d, +d), (1 - d, 1 + d), **kwargs)  # top-left diagonal
    ax2.plot((1 - d, 1 + d), (1 - d, 1 + d), **kwargs)  # top-right diagonal

    # Set labels and titles
    ax2.set_xlabel('Testcase', fontsize=8)
    ax1.set_ylabel('Time (s)', fontsize=8)
    ax2.set_ylabel('Time (s)', fontsize=8)

    # Set x-ticks and labels
    ax2.set_xticks(positions + bar_width)
    ax2.set_xticklabels(group_df['Test Name'], rotation=90, ha='right', fontsize=7)

    handles, labels = ax2.get_legend_handles_labels()
    # Append the timeout label to the legend
    handles.append(plt.Line2D([0], [0], color='r', linestyle='--'))
    labels.append(f'Timeout (≥ {timeout_value}s)')
    ax1.legend(handles, labels, fontsize=5, framealpha=0.5, loc='upper right')
    ax2.grid(axis='y', linestyle='--', linewidth=0.7)

    plt.tight_layout()
    plt.subplots_adjust(hspace=0.1)
    plt.savefig(f'runtime_{group_name}.pgf')
    plt.savefig(f'runtime_{group_name}.png')

def plot_group_no_broken_axis(group_name, group_df):
    # Process the group dataframe
    group_df['Test Name'] = group_df['Test Name'].apply(lambda x: x.split('/')[-1].replace('.ltlf', ''))
    group_df = group_df.sort_values(by='Test Name', key=lambda col: col.map(alphanum_key))
    print(group_df)

    # Define the positions after sorting
    positions = np.arange(len(group_df))

    # Determine the dynamic timeout value
    max_time = 0
    for i, (algorithm, df) in enumerate(data.items()):
        group_data = df[df['test_group'] == group_name].sort_values(by='Test Name', key=lambda col: col.map(alphanum_key))
        for j, (index, row) in enumerate(group_data.iterrows()):
            if row['complete time'] != timeout_value:
                max_time = max(max_time, row['complete time'])
    dynamic_timeout_value = max_time + 10
    dynamic_timeout_value = int((dynamic_timeout_value + 20)//10*10)
    print(f"Calculated dynamic timeout {dynamic_timeout_value}")

    # Create figure and axis
    fig, ax = plt.subplots(figsize=(14, 8))
    bar_width = 0.25

    # Plot the dynamic timeout line without adding it to the legend
    ax.axhline(y=dynamic_timeout_value, color='r', linestyle='--', zorder=1)

    # Get color palettes
    bright_colors = sns.color_palette("bright", len(data))
    pastel_colors = sns.color_palette("bright", len(data))

    # Plot data for each algorithm
    for i, (algorithm, df) in enumerate(data.items()):
        group_data = df[df['test_group'] == group_name].sort_values(by='Test Name', key=lambda col: col.map(alphanum_key))

        # Create an array to store positions for the current algorithm
        current_positions = positions + i * bar_width
        
        first_timeout = True  # Flag to add timeout to legend only once
        for j, (index, row) in enumerate(group_data.iterrows()):
            if row['complete time'] >= dynamic_timeout_value:
                ax.bar(current_positions[j], dynamic_timeout_value, width=bar_width, 
                       label=f"{algorithm} (timeout)" if first_timeout else "",
                       color=pastel_colors[i], alpha=0.5, hatch='//', edgecolor="white", zorder=2)
                first_timeout = False
            else:
                ax.bar(current_positions[j], row['complete time'], width=bar_width, 
                       label=f"{algorithm}" if j == 0 else "",
                       color=bright_colors[i], zorder=2)

    # Set labels and titles
    ax.set_xlabel('Testcase', fontsize=12)
    ax.set_ylabel('Time (s)', fontsize=12)
    ax.set_title(f'Performance Comparison of Algorithms - {group_name}', fontsize=16)

    # Set x-ticks and labels
    ax.set_xticks(positions + (len(data) - 1) * bar_width / 2)
    ax.set_xticklabels(group_df['Test Name'], rotation=90, ha='right', fontsize=8)

    # Hide y-ticks at the timeout line
    def custom_y_ticks(x, pos):
        if x >= dynamic_timeout_value:
            return ''
        return x
    ax.yaxis.set_major_formatter(plt.FuncFormatter(custom_y_ticks))

    # Update legend to remove duplicates and exclude the timeout line
    handles, labels = ax.get_legend_handles_labels()
    by_label = dict(zip(labels, handles))
    filtered_handles = [by_label[label] for label in by_label if 'Timeout' not in label]
    filtered_labels = [label for label in by_label if 'Timeout' not in label]
    ax.legend(filtered_handles, filtered_labels, fontsize=10, loc='upper left', bbox_to_anchor=(1, 1))

    ax.grid(axis='y', linestyle='--', linewidth=0.7)

    plt.tight_layout()
    plt.savefig(f'runtime_{group_name}.pgf')
    plt.savefig(f'runtime_{group_name}.png')
    #plt.show()

# Generate plots for each group
for group_name in ['hiker', 'sheep', 'trap', 'other']:
    if group_name != 'other':
        plot_group(group_name, data['mso'][data['mso']['test_group'] == group_name])
    else:
        plot_group_no_broken_axis(group_name, data['mso'][data['mso']['test_group'] == group_name])


# Perform Wilcoxon signed-rank test between pairs of algorithms
pairs = [('mso', 'direct'), ('mso', 'belief'), ('direct', 'belief')]
results = {}

# Perform Wilcoxon signed-rank test for each test group individually
for group_name in ['hiker', 'sheep', 'trap']:
    print(f"\nWilcoxon Signed-Rank Test Results for {group_name} test group:")
    for (algo1, algo2) in pairs:
        group_data1 = data[algo1][data[algo1]['test_group'] == group_name]['complete time']
        group_data2 = data[algo2][data[algo2]['test_group'] == group_name]['complete time']
        stat, p_val = wilcoxon(group_data1, group_data2)
        print(f"{algo1} vs {algo2}: statistic={stat}, p-value={p_val}")
        if p_val < 0.05:
            print(f"  -> Significant difference at alpha=0.05")
        else:
            print(f"  -> No significant difference at alpha=0.05")
for (algo1, algo2) in pairs:
    stat, p_val = wilcoxon(data[algo1]['complete time'], data[algo2]['complete time'])
    results[(algo1, algo2)] = (stat, p_val)

# Output the results of the Wilcoxon Signed-Rank Tests
print("\nWilcoxon Signed-Rank Test Results:")
for pair, (stat, p_val) in results.items():
    print(f"{pair[0]} vs {pair[1]}: statistic={stat}, p-value={p_val}")
    if p_val < 0.05:
        print(f"  -> Significant difference at alpha=0.05")
    else:
        print(f"  -> No significant difference at alpha=0.05")
