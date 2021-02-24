Bubblemon visualizes CPU + memory + IO load + battery charge as a bubbling
liquid.

![](pixmaps/macbookpro-with-bubblemon.png)

# Legend

The **water level** indicates how much RAM memory is in use.

The **color of the liquid** indicates how much swap space is used, between
**blue (none)** and **red (all)**.

The **amount of bubbles** reflects the CPU load. Bubbles **in the middle only**
means only one core is doing work. Bubbles **across the whole width** means all
cores are busy.

A **reed-like graph** shows IO load.

**Fog** means your battery is running low.

# Installing

```sh
curl https://raw.githubusercontent.com/walles/bubblemon/master/osx/install.sh | bash
```

# Developing

Go [here](https://github.com/walles/bubblemon/blob/master/osx/README.md) for
development instructions for macOS.
