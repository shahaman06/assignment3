import tkinter as tk
from tkinter import filedialog, messagebox
import csv
import random
from collections import defaultdict

DAYS = ["Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"]
SHIFTS = ["Morning", "Afternoon", "Evening"]

class Employee:
    def __init__(self, name):
        self.name = name
        self.preferences = {day: [] for day in DAYS}
        self.assigned_days = set()

class SchedulerApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Employee Scheduler with Priorities")

        self.employees = []
        self.schedule = {day: {shift: [] for shift in SHIFTS} for day in DAYS}
        self.build_gui()

    def build_gui(self):
        frame = tk.Frame(self.root)
        frame.pack(pady=10)

        tk.Button(frame, text="Import CSV", command=self.import_employees).grid(row=0, column=0, padx=5)
        tk.Button(frame, text="Generate Schedule", command=self.generate_schedule).grid(row=0, column=1, padx=5)

        self.output_text = tk.Text(self.root, width=100, height=30)
        self.output_text.pack(padx=10, pady=10)

    def import_employees(self):
        path = filedialog.askopenfilename(filetypes=[("CSV Files", "*.csv")])
        if not path:
            return
        try:
            with open(path, newline='') as f:
                reader = csv.DictReader(f)
                self.employees = []
                for row in reader:
                    name = row['Name'].strip()
                    if not name:
                        continue
                    emp = Employee(name)
                    for day in DAYS:
                        if row[day]:
                            entries = row[day].split(",")
                            ranked = sorted(entries, key=lambda x: int(x.strip().split(":")[0]))
                            prefs = [x.strip().split(":")[1] for x in ranked if ":" in x]
                            emp.preferences[day] = [s for s in prefs if s in SHIFTS]
                    self.employees.append(emp)
            messagebox.showinfo("Success", "CSV imported successfully.")
        except Exception as e:
            messagebox.showerror("Error", str(e))

    def generate_schedule(self):
        self.schedule = {day: {shift: [] for shift in SHIFTS} for day in DAYS}
        manager = Employee("Manager")
        manager.assigned_days = set()

        for emp in self.employees:
            emp.assigned_days = set()

        # Priority-based assignment
        for day in DAYS:
            shift_counts = defaultdict(int)
            for emp in self.employees:
                if len(emp.assigned_days) >= 5:
                    continue
                prefs = emp.preferences[day]
                for shift in prefs:
                    if shift_counts[(day, shift)] < 2 and emp.name not in self.schedule[day][shift]:
                        if day not in emp.assigned_days:
                            self.schedule[day][shift].append(emp.name)
                            emp.assigned_days.add(day)
                            shift_counts[(day, shift)] += 1
                            break

        # Fill missing shifts
        for day in DAYS:
            for shift in SHIFTS:
                while len(self.schedule[day][shift]) < 2:
                    available = [e for e in self.employees if day not in e.assigned_days and len(e.assigned_days) < 5]
                    if available:
                        chosen = random.choice(available)
                        self.schedule[day][shift].append(chosen.name)
                        chosen.assigned_days.add(day)
                    elif len(manager.assigned_days) < 5:
                        self.schedule[day][shift].append("Manager")
                        manager.assigned_days.add(day)
                        break
                    else:
                        self.schedule[day][shift].append("Unassigned")
                        break

        self.display_schedule(manager)

    def display_schedule(self, manager):
        self.output_text.delete("1.0", tk.END)
        manager_errors = []
        unassigned_errors = []

        for day in DAYS:
            self.output_text.insert(tk.END, f"{day}:\n")
            for shift in SHIFTS:
                names = self.schedule[day][shift]
                self.output_text.insert(tk.END, f"  {shift}: {', '.join(names)}\n")
                if names == ["Manager"]:
                    manager_errors.append(f"{day} - {shift}")
                elif "Unassigned" in names:
                    unassigned_errors.append(f"{day} - {shift}")
            self.output_text.insert(tk.END, "\n")

        if manager_errors or unassigned_errors:
            self.output_text.insert(tk.END, "\nERROR SUMMARY\n")

        if manager_errors:
            self.output_text.insert(tk.END, "\nOnly Manager available (requires more staff):\n")
            for err in manager_errors:
                self.output_text.insert(tk.END, f"• {err}\n")

        if unassigned_errors:
            self.output_text.insert(tk.END, "\nUnassigned Shifts (even Manager unavailable):\n")
            for err in unassigned_errors:
                self.output_text.insert(tk.END, f"• {err}\n")

# Launch the GUI
if __name__ == "__main__":
    root = tk.Tk()
    app = SchedulerApp(root)
    root.mainloop()
