# Special Topics

## Concerning Osu -> Beet Saber

Things osu has that can be hard to translate:

- Spinners:

    Can drop these: they normally happen during idle periods in a song anyways

- Skip section (long first-note delay)

    In a way these are desirable. Can drop these

- Repeat Sliders:

    This is important. FYI it is easy to parse whether a slider is a repeat slider (see "slider syntax" in the wiki). Can duplicate the slider n times, but that would require parsing TimingPoints in parallel, finding if the slider is inherited/uninherited, doing algebra to find beat-precise inserts, etc. Too much work; shelving for now



