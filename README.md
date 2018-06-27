# symptomBasedFaultDetection
    PreTests 1 (Bestimmung der Benchmarks) 

10 - Fache Messung der Ausführungszeit der Rodinia Benchmarks 

Danach Berechnung des Mittelwerts der Metrik Zeit 

Festlegung des Mittelwerts als Zielwert 

Festlegung des Toleranzbereiches (niedrigster Wert -  Zielwert - höchster Wert z.B.) 

Danach für jeden Fehler i aus der Menge der Gesamtfehler: 

10-Fache Messung der fehlerbehafteten Ausführungszeit  

Berechnung Mittelwert -> Berechnung der Abweichung  

Auswahl von maximal 5 Metriken (Benchmark + Fehler) für ausführliche Tests 

Grundfrage: Welcher Benchmark unterliegt der größten Wirkung von einem bestimmten Fehler? 

    PreTests 2 (Bestimmung der Symptome) 

Für jeden Fehler i: 

3-Fache Messung aller Symptome der normalen Ausführung 

Danach Bestimmung Mittelwert f. jedes Symptom clean 

Danach 3 -Fache Messung  aller Symptome der gestörten (Fehler i) Ausführung 

Danach Bestimmung Mittelwert f. jedes Symptom gestört 

Wähle die 5-10 Symptome mit der größten Abweichung der Mittelwerte 

    Test 

2-Fache Messung der cleanen Ausführungszeit, d.h. ohne Symptom/Fehler 

5-Fache Ausführung der Rodinia Benchmarks, Messung für jedes Symptom j 

Berechnung Mittelwerte und Toleranzbereich 

5-Fache fehlerbehaftete Ausführung der Rodinia Benchmarks, Messung für jedes Symptom j und Fehler i. 

    Interpretation d. Ergebnisse 

Für jeden Fehler i der gefunden werden kannn, werden möglichst viele Symptome angeschaut 

Vergleich der Symptome -> Ähnlichkeiten zu anderen Fehlern? Eindeutige Abgrenzung? 

Vergleich der Delta-Werte! 

 
