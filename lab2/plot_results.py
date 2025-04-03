import pandas as pd
import matplotlib.pyplot as plt

# Read CSV file

def plot_results(filename, title_name):
    # Read the CSV file
    df = pd.read_csv(filename)

    # Plot execution times
    plt.figure()
    plt.plot(df['Selectivity'], df['DirectTime'], marker='o', label='Direct Scan')
    plt.plot(df['Selectivity'], df['IndexTime'], marker='s', label='Index Based')
    plt.xlabel("Selectivity (fraction of rows selected)")
    plt.ylabel("Execution Time (ms)")
    plt.title(title_name + ": Range Query Execution Time vs. Selectivity")
    plt.legend()
    plt.savefig(title_name + "_execution_time.png")
    plt.show()

if __name__ == "__main__":
    # Example usage
    plot_results("build/test_P1_result.csv", "P1")
    plot_results("build/test_P2_result.csv", "P2")

