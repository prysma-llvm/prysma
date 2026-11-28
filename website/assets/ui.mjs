export const ui = {
  en:{
    docs:'Documentation', browse:'Browse docs', find:'Find a page', outline:'On this page', skip:'Skip to content',
    groups:{start:'Overview',setup:'Set up Prysma',language:'Language & compiler',project:'Project',operations:'Infrastructure'},
    source:'View source', page:'Overview', git:'Prysma on GitHub',
    names:{'installation/first_program.md':'First program','installation/docker_compilation.md':'Build with Docker','installation/vscode_environment.md':'VS Code and debugging','installation/setup_ubuntu.md':'Native Ubuntu build','prysma.md':'Language reference','architecture.md':'Compiler architecture','changelog.md':'Changelog','roadmap.md':'Roadmap','robustness.md':'Robustness and security'},
    loading:'Loading documentation…', latest:'Synced with GitHub', failed:'GitHub unavailable', cached:'Last known revision',
    empty:'No matching pages.', found:n=>n+' pages found', unavailable:'This page could not be loaded. Open the source on GitHub or return to the overview.',
    missing:'This page is not in the current documentation.', license:'GPL-3.0-or-later with Prysma exception',
    copy:'Copy', copied:'Copied',copyError:'Unavailable',table:'Scrollable table', navError:'Invalid navigation lines:', navUnavailable:'Navigation file unavailable',
    alerts:{note:'Note',tip:'Tip',important:'Important',warning:'Warning',caution:'Caution'},
  },
  fr:{
    docs:'Documentation', browse:'Parcourir la doc', find:'Trouver une page', outline:'Sur cette page', skip:'Aller au contenu',
    groups:{start:'Vue d’ensemble',setup:'Installer Prysma',language:'Langage et compilateur',project:'Projet',operations:'Infrastructure'},
    source:'Voir la source',page:'Vue d’ensemble',git:'Prysma sur GitHub',
    names:{'installation/first_program.md':'Premier programme','installation/docker_compilation.md':'Compiler avec Docker','installation/vscode_environment.md':'VS Code et débogage','installation/setup_ubuntu.md':'Compilation native sur Ubuntu','prysma.md':'Référence du langage','architecture.md':'Architecture du compilateur','changelog.md':'Journal de développement','roadmap.md':'Feuille de route','robustness.md':'Robustesse et sécurité'},
    loading:'Chargement de la documentation…',latest:'Synchronisé avec GitHub',failed:'GitHub indisponible',cached:'Dernière révision connue',
    empty:'Aucune page ne correspond.',found:n=>n+' pages trouvées',unavailable:'Impossible de charger cette page. Ouvre la source sur GitHub ou reviens à la vue d’ensemble.',
    missing:'Cette page ne figure pas dans la documentation actuelle.',license:'GPL-3.0-or-later avec exception Prysma',
    copy:'Copier',copied:'Copié',copyError:'Indisponible',table:'Tableau défilant', navError:'Lignes de navigation invalides :', navUnavailable:'Fichier de navigation indisponible',
    alerts:{note:'Note',tip:'Conseil',important:'Important',warning:'Avertissement',caution:'Attention'},
  },
};

export const priority = {'home':0,'installation/first_program.md':0,'installation/docker_compilation.md':1,
  'installation/vscode_environment.md':2,'installation/setup_ubuntu.md':3,'prysma.md':0,'architecture.md':1,'changelog.md':0,'roadmap.md':1,
  'installation/dashboard/guide.md':0};
