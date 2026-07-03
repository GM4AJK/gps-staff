import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import 'package:url_launcher/url_launcher.dart';
import '../theme.dart';
import '../widgets/status_bar.dart';
import 'about_screen.dart' show DarkHeaderBar;

// ── Data ──────────────────────────────────────────────────────────────────────

class _Link {
	const _Link({required this.title, required this.desc, required this.url});
	final String title;
	final String desc;
	final String url;
}

const _sections = [
	(
		label: 'SURVEY & MAPPING',
		links: [
			_Link(
				title: 'OS Net',
				desc: 'Ordnance Survey active network — NTRIP correction streams for GB',
				url: 'https://www.ordnancesurvey.co.uk/customers/public-sector/mapping-surveying/os-net',
			),
			_Link(
				title: 'OS Passive Stations Search',
				desc: 'Find benchmark coordinates for a fixed base position',
				url: 'https://www.ordnancesurvey.co.uk/geodesy-positioning/legacy-data/passive-search',
			),
			_Link(
				title: 'Trigpointing UK',
				desc: 'Community database of OS trig pillars — locations, photos and visit logs',
				url: 'https://trigpointing.uk/',
			),
			_Link(
				title: 'OS National Coordinate Transformation',
				desc: 'Verify ETRS89 → OSGB36/ODN conversions against the OS reference tool',
				url: 'https://www.ordnancesurvey.co.uk/geodesy-positioning/transformation-information/coordinate-transformation',
			),
			_Link(
				title: 'OSTN15/OSGM15 Developer Pack',
				desc: 'Download the grid binary needed for ETRS89 → OSGB36/ODN on-device transform',
				url: 'https://www.ordnancesurvey.co.uk/documents/resources/OSTN15-OSGM15-developers-pack.zip',
			),
		],
	),
	(
		label: 'NTRIP & RTK',
		links: [
			_Link(
				title: 'rtk2go',
				desc: 'Free public NTRIP caster — stream base corrections and consume them as a rover',
				url: 'http://rtk2go.com',
			),
			_Link(
				title: 'NTRIP Protocol (BKG)',
				desc: 'BKG NTRIP Client and server specification documents',
				url: 'https://igs.bkg.bund.de/ntrip/about',
			),
		],
	),
	(
		label: 'HARDWARE REFERENCES',
		links: [
			_Link(
				title: 'u-blox ZED-F9P',
				desc: 'Product page and integration documentation for the GNSS module',
				url: 'https://www.u-blox.com/en/product/zed-f9p-module',
			),
			_Link(
				title: 'Waveshare Core1262-LF',
				desc: 'LoRa transceiver module wiki (410–510 MHz variant)',
				url: 'https://www.waveshare.com/wiki/Core1262-868M',
			),
			_Link(
				title: 'Semtech SX1262 Product Page',
				desc: 'SX1262 transceiver datasheet and application notes',
				url: 'https://www.semtech.com/products/wireless-rf/lora-connect/sx1262',
			),
		],
	),
	(
		label: 'SPACE WEATHER',
		links: [
			_Link(
				title: 'NOAA Space Weather Prediction Center',
				desc: 'Real-time solar wind, Kp index, and geomagnetic storm alerts',
				url: 'https://www.swpc.noaa.gov',
			),
			_Link(
				title: 'BGS Geomagnetism — Hartland Observatory',
				desc: 'UK geomagnetic field data relevant to ionospheric TEC disturbances',
				url: 'https://geomag.bgs.ac.uk/data_service/data/observatory/hartland/',
			),
		],
	),
];

// ── Screen ────────────────────────────────────────────────────────────────────

class LinksScreen extends StatelessWidget {
	const LinksScreen({super.key});

	Future<void> _open(String url) async {
		final uri = Uri.parse(url);
		await launchUrl(uri, mode: LaunchMode.externalApplication);
	}

	@override
	Widget build(BuildContext context) {
		return Scaffold(
			backgroundColor: kBgHome,
			resizeToAvoidBottomInset: false,
			body: Column(
				crossAxisAlignment: CrossAxisAlignment.stretch,
				children: [
					const StatusBar(),
					DarkHeaderBar(
						title: 'Links',
						onBack: () => Navigator.pop(context),
					),
					Expanded(
						child: ListView(
							padding: const EdgeInsets.fromLTRB(24, 16, 24, 24),
							children: [
								for (final section in _sections) ...[
									Padding(
										padding: const EdgeInsets.only(bottom: 8, top: 4),
										child: Text(
											section.label,
											style: GoogleFonts.montserrat(
												fontSize: 10,
												fontWeight: FontWeight.w700,
												color: kTextMuted,
												letterSpacing: 1.2,
											),
										),
									),
									for (final link in section.links) ...[
										_LinkRow(link: link, onTap: () => _open(link.url)),
										const Divider(height: 1, thickness: 1, color: Color(0xFF1A2535)),
									],
									const SizedBox(height: 20),
								],
							],
						),
					),
				],
			),
		);
	}
}

class _LinkRow extends StatelessWidget {
	const _LinkRow({required this.link, required this.onTap});
	final _Link link;
	final VoidCallback onTap;

	@override
	Widget build(BuildContext context) {
		return GestureDetector(
			onTap: onTap,
			behavior: HitTestBehavior.opaque,
			child: Padding(
				padding: const EdgeInsets.symmetric(vertical: 14),
				child: Row(
					children: [
						Expanded(
							child: Column(
								crossAxisAlignment: CrossAxisAlignment.start,
								children: [
									Text(
										link.title,
										style: GoogleFonts.montserrat(
											fontSize: 15,
											fontWeight: FontWeight.w600,
											color: kTextPrimary,
										),
									),
									const SizedBox(height: 3),
									Text(
										link.desc,
										style: GoogleFonts.montserrat(
											fontSize: 12,
											color: kTextSub,
											height: 1.4,
										),
									),
								],
							),
						),
						const SizedBox(width: 12),
						Text(
							'›',
							style: GoogleFonts.montserrat(
								fontSize: 22,
								color: kTextMuted,
							),
						),
					],
				),
			),
		);
	}
}
