# Special Topics

## Concerning Osu -> Beet Saber

Things osu has that can be hard to translate:

- Spinners:

    Can drop these: they normally happen during idle periods in a song anyways

- Skip section (long first-note delay)

    In a way these are desirable. Can drop these

- Repeat Sliders:

    This is important. FYI it is easy to parse whether a slider is a repeat slider (see "slider syntax" in the wiki). Can duplicate the slider n times, but that would require parsing TimingPoints in parallel, finding if the slider is inherited/uninherited, doing algebra to find beat-precise inserts, etc. Too much work; shelving for now

## Human Capabilities in 3D space

### How Lenient Should Hit Windows Be?

In osu, OD4 is $\pm 56$ms for perfects and $\pm 170$ for any valid inputs. Accordingly, can do $\pm 60$ for perfect/good/ok (symmetry). This seems reasonable. Total window is $360$ms long for note inputs.

Algebra:

- Each pixel row: $60$ms (2 pixels each for perfect,good,ok zones) (this is also almost guaranteed to prevent note overlap for 3x3 note)
- Each pixel board can store $60*32ms = 1.98s$ of content (that's a good amnt)