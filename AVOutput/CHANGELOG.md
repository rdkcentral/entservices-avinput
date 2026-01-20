# Changelog

All notable changes to this RDK Service will be documented in this file.

* Each RDK Service has a CHANGELOG file that contains all changes done so far. When version is updated, add a entry in the CHANGELOG.md at the top with user friendly information on what was changed with the new version. Please don't mention JIRA tickets in CHANGELOG. 

* Please Add entry in the CHANGELOG for each version change and indicate the type of change with these labels:
    * **Added** for new features.
    * **Changed** for changes in existing functionality.
    * **Deprecated** for soon-to-be removed features.
    * **Removed** for now removed features.
    * **Fixed** for any bug fixes.
    * **Security** in case of vulnerabilities.

* Changes in CHANGELOG should be updated when commits are added to the main or release branches. There should be one CHANGELOG entry per JIRA Ticket. This is not enforced on sprint branches since there could be multiple changes for the same JIRA ticket during development. 

## [1.1.4] - 2025-09-08
### Fixed
- Sync low latency for AVOutput initialization

## [1.1.3] - 2025-08-08
### Fixed
- Fixed getDolbyVisionMode() for non DV content

## [1.1.2] - 2025-07-01
### Fixed
- Phase2 ODM

## [1.1.1] - 2025-06-26
### Fixed
- Fixed dimmingMode failures

## [1.1.0] - 2025-03-14
### Added
- Add additional features on AVOutput

## [1.0.10] - 2025-02-17
### Fixed
ODM API removal changes phase 1 and Fixed PQ Mode Camel Case issue

## [1.0.0] - 2025-02-17
### Added
- Add CHANGELOG

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
