Report Guidelines and Advice
======================================================================

As you learn to write code,
it is also important to learn how to write _about_ your code.
The reports you write alongside your assignments are a good chance
to practice this kind of writing.

Report Guidelines for INF-2203, Spring 2026
==================================================

- **Structure: scientific paper**

    Your report should follow the basic structure of a scientific paper:
    introduce the system you built, review background concepts, describe the
    system, describe how it was evalutated, and describe the results of that
    evaluation.
    See below for a more detailed structure suggestion.

- **Length: around 4 pages at 10pt font size**

    Four pages with 10pt font is the recommended
    length and font size for your report.
    This is not a hard requirement. You may go over or go under,
    but if your report is much shorter or much longer,
    then it is probably a sign that you need to either add more information
    or trim some fat.

- **Citations: required**

    You must cite your sources.
    You will not necessarily have to do a lot of research in this class,
    but when you talk about operating systems concepts in your report,
    we expect you to cite the source of your information,
    even if it is just the textbook.

- **File format: PDF**

    Your finished report should be in PDF format.
    If you are using a word processor like Word,
    export to PDF before hand-in.
    If you are using a typesetting language like LaTeX,
    render to PDF before hand-in.

- **AI tools: VERY STRONGLY DISCOURAGED**

    In this course we strongly discourage you from using LLMs or other
    generative AI tools to help you write your code or your reports.
    We have main three points we want to stress:

    1. These assignments are meant to give you experience and practice,
        to help you learn how to actually _do_ this work.
        If you let AI do the work for you,
        then you are cheating yourself.

    2. **If you can't be bothered to write it,
        then we don't want to waste our time reading it.**

    3. If you do use AI tools to assist you,
        then you MUST add a section to your report
        that explains what tools you used and how you used them.
        **Failure to disclose AI use will be considered plagiarism and
        may be reported to the administration as cheating.**

Assignment Hand-In Instructions for INF-2203, Spring 2026
==================================================

1. **Clean your code tree.**
    Run `make clean` to delete built files for a smaller upload file size.

2. **Zip/tar up your repository and upload it to Canvas.**
    Please include the full Git repository,
    including commit history, in your upload
    (i.e. include the hidden `.git/` directory
     that stores history and metadata).

3. **Upload your report PDF to Canvas as a separate file.**
    Please convert your report to PDF and upload the PDF to Canvas
    separately from your code zip/tarball.
    This makes it easier for the TAs to check and make comments on your report.

Report Layout Templates
==================================================

IEEE Style
--------------------------------------------------

We like the style that IEEE uses for their papers.
IEEE publishes layout templates for both MS Word and LaTeX
that are free to use.
We recommend trying them out.

<https://www.ieee.org/conferences/publishing/templates>

Using IEEE style is not a hard requirement.
If you have an alternate scientific paper template that you prefer,
that is ok, as long as it looks professional.

TD Templates
--------------------------------------------------

[Tromsøstudentenes Dataforening (TD)](https://td-uit.no/)
maintains assignment templates for
both LaTeX and [Typst](https://typst.app/home)
on their GitHub page.
You may find those useful.

<https://github.com/td-org-uit-no>

Suggested Scientific Paper Structure
==================================================

You should structure your reports like a scientific paper.
The correct structure for a scientific paper depends on the field, subfield,
and even the specific project.
But for systems research in computer science,
the following structure tends to work well.
We recommend starting with an outline like this
and then adapting it to suit your needs.

- **Abstract**

    One or two paragraphs that summarize your entire paper: the problem you are
    solving, the short version of how you solved it, and what your conclusions
    are.

1. **Introduction**

    A brief overview of your system and the fundamental problem it is trying to
    solve.

2. **Background**

    A brief overview of concepts necessary to understand the problem and your
    solution.
    For the boot assignment this might be an overview of the boot protocol
    For the system call assignment it might be an overview of what system
    calls are and how the CPU enforces the separation of kernel code from
    user-space code.

3. **Related Work**

    A compact review of other work in your project's specific subfield.
    This is where you cite most of the things that you read while researching
    your project. What other projects inspired yours? What are you building on?
    What are you competing with?

    In this course you will not be doing a great deal of research,
    but if you, say, read up on how Linux does scheduling before you
    implemented your own scheduler, this is the place to write about that.

4. **System Description**

    Actually describe the system you built.
    We like to describe a system at four different layers of abstraction:

    1. **Idea:**
        Start with an extremely high-level "elevator pitch" for your system.
        What is the main idea that motivates it and sets it apart from similar
        existing systems?

        Since the assignments have relatively fixed requirements,
        you will probably not have a lot to write here.
        You can omit this subsection or you can restate the requirements,
        i.e. the problem you are trying to solve.

    2. **Architecture:**
        Then begin describing the system itself, still at a very high level.
        What are its major components and how do they connect?
        What are the most essential factors that shaped the highest-level
        structure of your system?

    3. **Design:**
        Now go a little deeper, but not too detailed.
        What are the smaller components inside the major ones?
        What factors shaped your design decisions as you fleshed out the
        architecture?
        Give pseudocode for important algorithms or protocols that make your
        system unique.
        At this level, someone reading your report should be able to write
        a similar system in a different programming language or for a different
        architecture.

    4. **Implementation:**
        Finally, describe implementation details such as programming language
        used and target hardware.

5. **Evaluation**

    Describe the methods used to evaluate the system. Define the
    metrics used to measure performance, and the procedures used to
    gather metrics.
    Include details like the specs of the hardware that ran the experiments.
    Do not include results yet.
    Those go in the next section.

6. **Results**

    Give the results of the evaluation, including data tables and plots.
    Describe your results in a very concrete, quantitative way.
    Just the facts.
    Qualitative discussion can go in the next section.

7. **Discussion**

    Discuss your solution and your experiment results in a more
    qualitative way.
    What are the positives and negatives?
    Discuss unexpected results and their implications.

8. **Future Work**

    Discuss possible ways to improve or expand the system.
    What would you want to do if you had more time?
    Are there any bugs that you didn't manage to fix in time?
    If so, discuss the bugs and how you would try to fix them here.

9. **Conclusion**

    Briefly restate the key points of your work and wrap up the paper.

Since the parameters of your assignments in this course are relatively narrow,
you may not have a lot of content for every section listed.
In that case you may shorten or combine sections,
but try to follow the spirit of the template.

Citations and Sources
==================================================

Of course, you must cite the sources of information that you include
in your report.
We encourage you to do a little more research into the concepts and systems
that we are studying and implementing, and to include that research in your
report.

When you first search for information,
you will probably first find overviews like Wikipedia and tutorial sites.
It is fine to use these to get started in learning about a topic,
but you should try to dig beyond such sites to more authoritative sources.
Where did the tutorial page get their information?
Luckily, Wikipedia articles for computer science concepts often have
links to the original papers that described or introduced them.
Follow those links!

Also, when you read a paper, look closely at the papers they cite.
A well-written pair of Background and Related Works sections will be a crash
course in the state of the art of the topic at the time the paper was
written.
Follow those links!

## Source Quality

- Best: peer-reviewed academic sources

    The top publishers of computer science research are
    [USENIX](https://www.usenix.org/),
    [the Association for Computing Machinery (ACM)](https://dl.acm.org/),
    and
    [the Institute of Electrical and Electronics Engineers (IEEE)](https://ieeexplore.ieee.org/Xplore/home.jsp).
    They have vast libraries of computer science papers that you
    can access from the campus networks.

- Good: books

- Good: well-known newspapers/magazines,
    especially if they have well-maintained online archives

- Useful as side sources: interviews or blogs from notable people

- Sloppy: Wikipedia, tutorial pages

Beware of link rot. Look for DOIs.

- Beware of URLs that may change or disappear over time,
  such as the homepage of a company or software project.

- Most modern peer-reviewed papers will have a
  [_Digital Object Identifier (DOI)_](https://en.wikipedia.org/wiki/Digital_object_identifier)
  that is meant to be a permanent identifier for that paper
  and a link to a reliable archive.
  If you find a DOI, use that as the link.

The ideals to look for in a source are:

- [x] Peer reviewed
- [x] Unchanging
- [x] Reliably archived, preferably with a DOI

Advice on Writing in General
==================================================

The short paper
"The ABC of academic writing: non-native speakers' perspective"
([doi:10.1016/j.tree.2024.01.008](https://doi.org/10.1016/j.tree.2024.01.008))
offers some concise writing advice that is especially geared towards
those who are not native English speakers.
This paper comes from the field of ecology,
but the general advice for writing should also apply well to computer science.

Note that the paper recommends bouncing ideas off an AI tool like ChatGPT.
Remember that WE STRONGLY DISCOURAGE AI USE in this course,
and remember that if you do use an LLM/AI tool,
YOU MUST DISCLOSE AI USE and explain how you used it.

Optional: Typesetting with LaTeX
==================================================

As a programmer,
you might enjoy using a typesetting language like LaTeX
instead of a WYSIWYG ("What You See Is What You Get") tool like MS Word.

LaTeX is an old-school typesetting language that is still the gold standard
for typesetting mathematical formulas.
Because of this, it is the de-facto standard for typesetting
scientific papers in the fields of computer science, mathematics, and physics.

LaTeX also allows you to treat your documents like code:
writing with your favorite text editor and tracking changes with Git.
As Mike likes to say:

> LaTeX is a great way to program about writing while you're procrastinating on
> writing about programming. --- Mike Murphy

The core TeX typesetting engine and macro language were developed by
legendary computer scientist and mathematician
[Donald Knuth](https://en.wikipedia.org/wiki/Donald_Knuth).
LaTeX is actually a comprehensive set of TeX macros,
developed by legendary distributed-systems researcher
[Leslie Lamport](https://en.wikipedia.org/wiki/Leslie_Lamport).
The LaTeX macros make it easier to separate content from style,
much like CSS does for the web.
These days it is relatively rare to use plain TeX without LaTeX.
Plain TeX is probably best left to the hard-core
[TeXnicians](https://tex.stackexchange.com/a/474039/229926).

TeX and LaTeX come from an earlier age of computer science,
and their syntax and semantics can seem arcane at times.
But due to their prevalence in academia,
they are very much worth learning,
especially if you intend to go on to a Master's or PhD.

TeX/LaTeX is available on most operating systems,
typically in a distribution called [TeXLive](https://www.tug.org/texlive/).
On Ubuntu/Debian, for example,
one just has to `apt install texlive`
to install most of the software you need.

If you are interested,
IEEE offers a
[LaTeX template](https://www.ieee.org/conferences/publishing/templates)
that can help you get started,
plus a separate PDF with
[instructions for using the template](http://www.ctan.org/tex-archive/macros/latex/contrib/IEEEtran/IEEEtran_HOWTO.pdf).

From there, it is a matter of learning how to use LaTeX.
There is a wealth of information online.
Good places to start include:

- [The LaTeX Wikibook](https://en.wikibooks.org/wiki/LaTeX)

    An excellent, comprehensive, and free crowdsourced guide to LaTeX

- [Overleaf's LaTeX documentation](https://www.overleaf.com/learn)

    Overleaf is a popular web-based LaTeX editor and collaboration tool.
    It is a commercial service that offers free or paid accounts on its
    servers,
    but it is also possible to download the
    [open-source core](https://github.com/overleaf/overleaf)
    of their platform
    software and host your own Overleaf instance.
    It is in Overleaf's interest to get more people using LaTeX,
    so they have put a lot of resources into thorough LaTeX documentation
    and tutorials,
    including nice quick-start guide
    [Learn LaTeX in 30 Minutes](https://www.overleaf.com/learn/latex/Learn_LaTeX_in_30_minutes)

- [TeX/LaTeX StackExchange](https://tex.stackexchange.com/)

    The StackOverflow/StackExchange family of Q&A sites includes one dedicated
    to TeX and LaTeX.

- [texfaq.org](https://texfaq.org/)

    A repository of frequently-asked questions and answers

- [texdoc.org](https://texdoc.org/)

    A repository of TeX package documentation

- [TeX User's Group (TUG)](https://tug.org/)

    The non-profit group that is the steward of TeX and LaTeX's development.
    They have many links to tools and documentation, and their journal
    [TUGboat](https://tug.org/TUGboat/)
    is the hub of TeX research and development.
