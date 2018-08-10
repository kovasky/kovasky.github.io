module.exports = {
  title: 'Kovasky Buezo',
  header: "Hi, I am Kovasky Buezo",
  description: "I am a Computer Engineering student at Memorial University of Newfoundland. My passions include computer architecture, hardware design and bio-energy.",
  name: 'Kovasky Buezo',
  footerTitle: 'Computer Engineering Student',
  email: 'kovasky.buezo@mun.ca',
  cta: {
    label: 'Get my resume',
    url: 'resume.pdf',
  },
  socials: [
    { icon: 'github-circled', url: 'https://www.github.com/kovasky' },
    { icon: 'mail-alt' }, // mail alt automatically links to mailto:email
    { icon: 'linkedin', url: 'htpps://www.linkedin.com/in/kovaskybuezo' },
  ],
  researchs: [
    {
      name: '',
      description: '[1] Ammar Musbah Saleh Ahmed, <b>Kovasky Buezo</b>, Noori Saady, <q>Adapting anaerobic consortium to pure and complex lignocellulose substrates at low temperature: Kinetics evaluation,</q> <em>International Journal of Recycling of Organic Waste in Agriculture,</em> 2018, under review.'
    },
    {
	    name: '',
	    description: '[2] Noori Saady, Carlos Bazan, <b>Kovasky Buezo</b>, <q>Anaerobic digestion of blood from slaughtered livestock animals: A review,</q> <em>Biomass and Bioenergy,</em> 2018, under review.'
    },
  ],

  projects: [
    {
      name: 'Chip-8 interpreter',
      description: 'Combustion is a sleek, modern web client for the transmission bittorrent client.',
      tags: ['C++'],
      alt: '', // alt description of image for accessibility. defaults to '{{name}} Screenshot'
      img: '',
      url: 'https://www.github.com/kovasky/chip8'
    },
  ],
    experiences: [
    {
      title: 'Associate Maintenance Engineer Co-op Student',
      timeline: 'February - April 2018',
      description: 'Empire Electronics, Honduras',
      responsibilities: ['Assisted in the designing of maintenance strategies, procedures and methods which reduced assembly line down-time to 3% and scrap production to 1%', 'Carried out routine scheduled maintenance work and responded to equipment faults', 'Diagnosed breakdown problems through the collection and analysis of on-site data']
    },
    {
      title: 'Associate Planning Engineer Co-op Student',
      timeline: 'February - April 2017',
      description: 'Sula Valley Biogas, Honduras',
      responsibilities: [
	      'Assisted in the planning, budgeting and decision-making process of the largest biogas energy company in Central America', 'Interpreted data from SCADA systems to create weekly graphs and reports pertinent to energy production and biomass consumption for the use of upper management','Wrote the first plant operations handbook after becoming familiar with the production process' 
      ]
    }
  ]
  }
